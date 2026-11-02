require 'fileutils'
require 'pathname'

root = File.expand_path('..', __dir__)
Dir.chdir(root)
tests = Dir['tests/*_test.c'].map { |path| File.basename(path, '.c') }.sort
binaries = ['assemblyArchitect', 'build/sanitize/assemblyArchitect'] +
           tests.flat_map { |name| ["build/tests/#{name}", "build/sanitize/tests/#{name}"] }
artifacts = binaries.flat_map { |path| [path, "#{path}.dSYM"] }.freeze

artifacts.each do |path|
  parts = Pathname.new(path).each_filename.to_a
  abort "Unsafe artifact path: #{path}" if Pathname.new(path).absolute? ||
    parts.include?('..') || path.match?(/[\*?\[\]]/) || path.end_with?('.swp')
  ancestor = File.dirname(path)
  until ancestor == '.'
    abort "Symlinked output ancestor: #{ancestor}" if File.symlink?(ancestor)
    ancestor = File.dirname(ancestor)
  end
  if path.end_with?('.dSYM')
    abort "Unexpected debug artifact: #{path}" if File.exist?(path) &&
      !File.directory?(path) && !File.symlink?(path)
  elsif File.directory?(path) && !File.symlink?(path)
    abort "Refusing directory in executable slot: #{path}"
  end
end

case ARGV
when ['--check']
  abort 'No test artifacts discovered' if tests.empty?
  abort 'Duplicate artifact paths' unless artifacts.uniq == artifacts
  puts "Cleanup contract checked: #{artifacts.length} explicit binary/debug paths; no deletion"
when ['--preview']
  puts 'Preview only. App bundle, asset symlinks, legacy root tests, saves and editor files are preserved.'
  artifacts.each { |path| puts "#{File.exist?(path) || File.symlink?(path) ? 'present' : 'absent'} #{path}" }
when ['--execute']
  artifacts.each do |path|
    if File.symlink?(path)
      File.unlink(path)
    elsif File.directory?(path)
      FileUtils.remove_entry_secure(path)
    elsif File.file?(path)
      File.unlink(path)
    end
  end
else
  abort 'Usage: ruby scripts/clean_artifacts.rb --check|--preview|--execute'
end
