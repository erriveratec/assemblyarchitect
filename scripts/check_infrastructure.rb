require 'yaml'
require 'open3'
require 'tmpdir'
require_relative 'test_asset_links'

Dir.chdir(File.expand_path('..', __dir__))
def require_check(condition, message)
  abort message unless condition
end

framework_script = File.read('scripts/ci_frameworks.sh')
framework_table = framework_script.match(/done <<'FRAMEWORKS'\n(.*?)^FRAMEWORKS$/m)
require_check(framework_table, 'Missing framework checksum table')
framework_entries = framework_table[1].lines.map(&:split)
expected_frameworks = [%w[SDL 2.32.0 SDL2], %w[SDL_image 2.8.5 SDL2_image],
                       %w[SDL_ttf 2.24.0 SDL2_ttf], %w[SDL_mixer 2.8.1 SDL2_mixer]]
require_check(framework_entries.map { |entry| entry.first(3) } == expected_frameworks,
              'Unexpected pinned framework inventory')
require_check(framework_entries.all? { |entry| entry.length == 4 && entry[3].match?(/\A[0-9a-f]{64}\z/) },
              'Each framework requires one lowercase 64-character SHA-256')

Dir.mktmpdir('assemblyarchitect-link-contract-') do |temporary|
  root = File.join(temporary, 'checkout')
  FileUtils.mkdir_p(TestAssetLinks::ASSETS.map { |name| File.join(root, name) })
  %w[build/tests build/sanitize/tests].each do |output|
    TestAssetLinks.setup(output, root: root)
    link = Pathname.new(root).join(output, 'data')
    original_inode = link.lstat.ino
    TestAssetLinks.setup(output, root: root)
    require_check(link.lstat.ino == original_inode, 'Valid asset link was replaced')
    link.unlink
    link.make_symlink(Pathname.new(root).join('data').realpath)
    original_inode = link.lstat.ino
    TestAssetLinks.setup(output, root: root)
    require_check(link.lstat.ino == original_inode, 'Valid existing absolute link was replaced')
    link.unlink
    link.make_symlink('/missing-old-checkout/data')
    TestAssetLinks.setup(output, root: root)
    require_check(link.readlink.relative? && link.realpath == Pathname.new(root).join('data').realpath,
                  'Dangling asset link was not repaired')
    link.unlink
    [false, true].each do |directory|
      directory ? link.mkdir : link.write('preserve this file')
      refused = false
      begin
        TestAssetLinks.setup(output, root: root)
      rescue ArgumentError
        refused = true
      end
      require_check(refused && (directory ? link.directory? : link.read == 'preserve this file'),
                    'Non-symlink asset slot was not preserved and refused')
      directory ? link.rmdir : link.unlink
    end
    link.make_symlink(Pathname.new(root).join('img').relative_path_from(link.dirname))
    refused = false
    begin
      TestAssetLinks.setup(output, root: root)
    rescue ArgumentError
      refused = true
    end
    require_check(refused, 'Wrong live asset link was accepted')
    link.unlink
    TestAssetLinks.setup(output, root: root)
  end
  moved = File.join(temporary, 'moved-checkout')
  File.rename(root, moved)
  %w[build/tests build/sanitize/tests].each do |output|
    original_inodes = TestAssetLinks::ASSETS.map { |name| Pathname.new(moved).join(output, name).lstat.ino }
    TestAssetLinks::ASSETS.each do |name|
      link = Pathname.new(moved).join(output, name)
      require_check(link.readlink.relative? && link.realpath == Pathname.new(moved).join(name).realpath,
                    'Asset links failed checkout relocation')
    end
    TestAssetLinks.setup(output, root: moved)
    require_check(TestAssetLinks::ASSETS.map { |name| Pathname.new(moved).join(output, name).lstat.ino } == original_inodes,
                  'Relocated valid links were replaced')
  end
end

Dir.mktmpdir('assemblyarchitect-checksum-contract-') do |temporary|
  commands = File.join(temporary, 'commands')
  FileUtils.mkdir_p(commands)
  doubles = {
    'uname' => "printf 'Darwin\\n'\n",
    'curl' => "printf 'download\\n' >> \"$CALL_LOG\"\n",
    'shasum' => <<~'SH',
      printf 'hash\n' >> "$CALL_LOG"
      if [[ "$CHECKSUM_MODE" == mismatch ]]; then
        printf '%064d  %s\n' 0 "$3"
      else
        while read -r repository version framework digest; do
          if [[ "$3" == */"${framework}-${version}.dmg" ]]; then
            printf '%s  %s\n' "$digest" "$3"
            exit 0
          fi
        done < "$DIGEST_TABLE"
        exit 1
      fi
    SH
    'hdiutil' => <<~'SH',
      if [[ "$1" == attach ]]; then
        printf 'attach\n' >> "$CALL_LOG"
        mkdir -p "$RUNNER_TEMP/assemblyarchitect-framework-mount/SDL2.framework" \
          "$RUNNER_TEMP/assemblyarchitect-framework-mount/SDL2_image.framework" \
          "$RUNNER_TEMP/assemblyarchitect-framework-mount/SDL2_ttf.framework" \
          "$RUNNER_TEMP/assemblyarchitect-framework-mount/SDL2_mixer.framework"
      fi
    SH
    'ditto' => "exit 0\n"
  }
  doubles.each do |name, body|
    path = File.join(commands, name)
    File.write(path, "#!/bin/bash\nset -euo pipefail\n#{body}")
    File.chmod(0700, path)
  end
  table = File.join(temporary, 'digests')
  File.write(table, framework_table[1])
  %w[success mismatch missing malformed].each do |mode|
    script = File.join(temporary, "#{mode}.sh")
    content = framework_script
    content = content.sub(framework_entries.first[3], '') if mode == 'missing'
    content = content.sub(framework_entries.first[3], 'not-a-digest') if mode == 'malformed'
    File.write(script, content)
    log = File.join(temporary, "#{mode}.log")
    environment = { 'PATH' => "#{commands}:#{ENV.fetch('PATH')}", 'RUNNER_TEMP' => temporary,
                    'GITHUB_ENV' => File.join(temporary, 'environment'), 'CALL_LOG' => log,
                    'CHECKSUM_MODE' => mode, 'DIGEST_TABLE' => table }
    output, result = Open3.capture2e(environment, 'bash', script)
    calls = File.exist?(log) ? File.readlines(log, chomp: true) : []
    if mode == 'success'
      require_check(result.success? && calls == %w[download hash attach] * 4 &&
                    output.scan('SHA-256 verified').length == 4, 'Checksum success/order contract failed')
    else
      require_check(!result.success? && !calls.include?('attach'), "#{mode}: reached DMG attach")
      require_check(calls.empty?, "#{mode}: downloaded invalid metadata") unless mode == 'mismatch'
    end
  end
end
puts 'Bootstrap correction contracts passed: four digests, offline mount guards, portable links and relocation'

agents = Dir['.github/agents/*.agent.md']
require_check(agents.length == 5, 'Expected five agents')
agents.each do |path|
  content = File.read(path)
  require_check(content.start_with?("---\n"), "No frontmatter: #{path}")
  metadata = YAML.safe_load(content.split('---', 3)[1])
  require_check((metadata.keys - %w[name description tools]).empty?, "Unsupported keys: #{path}")
  require_check(metadata['description'].is_a?(String) && !metadata['description'].empty?, "No description: #{path}")
  tools = metadata.fetch('tools')
  require_check((tools - %w[read search edit execute]).empty?, "Unsupported tools: #{path}")
  if path.match?(/planner|reviewer/)
    require_check(tools.sort == %w[read search], "Read-only violation: #{path}")
  end
  content.scan(/`((?:docs\/|\.github\/)[^`]+\.md)`/).flatten.each do |reference|
    require_check(File.file?(reference), "Missing agent path #{reference}")
  end
end

Dir['docs/{architecture,governance,refactor}/**/*.md'].each do |path|
  File.read(path).scan(/\]\(([^)#]+)(?:#[^)]*)?\)/).flatten.each do |link|
    require_check(File.exist?(File.expand_path(link, File.dirname(path))), "Broken link #{path}: #{link}")
  end
end

makefile = File.read('Makefile')
test_names = Dir['tests/*_test.c'].map { |path| File.basename(path, '_test.c').tr('_', '-') }.sort
discovered, result = Open3.capture2('make', '-n', 'test')
require_check(result.success?, 'make -n test failed')
executed = discovered.scan(%r{^\./build/tests/([^\s]+)_test$}).flatten.map { |name| name.tr('_', '-') }.sort
require_check(executed == test_names, 'Test source/recipe execution inventory mismatch')
require_check(makefile.include?("clean:\n\truby scripts/clean_artifacts.rb --execute"), 'Unexpected clean recipe')
require_check(!makefile.match?(/rm -rf|assemblyArchitect\*|\.swp|find .*dSYM/), 'Unsafe Make cleanup')

Dir['.github/workflows/*.{yml,yaml}', '.github/ISSUE_TEMPLATE/*.{yml,yaml}'].each do |path|
  workflow = YAML.safe_load(File.read(path))
  require_check(workflow.is_a?(Hash), "Invalid YAML object: #{path}")
  next unless path.include?('/workflows/')
  events = workflow['on'] || workflow[true]
  require_check(events, "Missing workflow triggers: #{path}")
  if path == '.github/workflows/nightly.yml'
    require_check(events.key?('schedule') && events.key?('workflow_dispatch'), 'Nightly triggers changed')
    require_check(workflow.dig('jobs', 'analysis', 'if') == "github.event_name == 'workflow_dispatch'",
                  'Known-failing static analysis must remain manual-only until bounded triage')
    require_check(!workflow.dig('jobs', 'headless-sanitizers', 'if'), 'Healthy nightly checks must remain scheduled')
  end
  require_check(!events.to_s.include?('pull_request_target'), "Unsafe event: #{path}")
  require_check(workflow.dig('permissions', 'contents') == 'read', "Contents permission: #{path}")
  require_check(!File.read(path).include?('secrets.'), "Secret dependency: #{path}")
  workflow.fetch('jobs').each_value do |job|
    require_check(job['timeout-minutes'].is_a?(Integer), "Missing timeout: #{path}")
    job.fetch('steps').each do |step|
      if step['uses'] && !step['uses'].start_with?('./')
        require_check(step['uses'].match?(/@(?:v\d+|[0-9a-f]{40})\z/), "Unpinned action: #{path}")
      elsif step['uses']
        require_check(File.file?(File.join(step['uses'], 'action.yml')), "Missing local action: #{path}")
      end
      require_check(!step['continue-on-error'], "Hidden failure: #{path}")
      step.fetch('run', '').scan(%r{(?:ruby|bash) (scripts/[\w./-]+)}).flatten.each do |reference|
        require_check(File.file?(reference), "Missing script #{reference}")
      end
      step.fetch('run', '').scan(/\bmake\s+(?:-B\s+)?([a-z][a-z0-9-]*(?:\s+[a-z][a-z0-9-]*)*)/).flatten.each do |targets|
        output, target_result = Open3.capture2e('make', '-n', *targets.split)
        require_check(target_result.success?, "Invalid workflow Make target: #{targets}\n#{output}")
      end
    end
  end
end
if File.file?('.github/workflows/copilot-setup-steps.yml')
  setup = YAML.safe_load(File.read('.github/workflows/copilot-setup-steps.yml'))
  require_check(setup['name'] == 'Copilot Setup Steps', 'Unexpected setup workflow name')
  require_check(setup.fetch('jobs').keys == ['copilot-setup-steps'], 'Required setup job ID missing')
end
if File.file?('.github/ISSUE_TEMPLATE/refactor-stage.yml')
  form = YAML.safe_load(File.read('.github/ISSUE_TEMPLATE/refactor-stage.yml'))
  fields = form.fetch('body').reject { |field| field['type'] == 'markdown' }
  require_check(fields.length == 14, 'Missing issue requirements')
  require_check(fields.all? { |field| field.dig('validations', 'required') }, 'Optional required issue field')
  require_check(fields.map { |field| field['id'] }.uniq.length == fields.length, 'Duplicate issue IDs')
end
puts "Infrastructure passed: #{agents.length} profiles, #{test_names.length} tests, YAML, links and cleanup contract"
