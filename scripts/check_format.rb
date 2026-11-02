root = File.expand_path('..', __dir__)
Dir.chdir(root)
source_files = Dir['{source,tests}/**/*.{c,h}'].sort
text_files = Dir['.github/**/*.{md,yml,yaml}', 'docs/{architecture,governance,refactor}/**/*.md',
                 'scripts/*.{rb,sh}', 'Makefile'].sort
abort 'No C source/header files found' if source_files.empty?
errors = []
(source_files + text_files).uniq.each do |path|
  content = File.binread(path)
  errors << "#{path}: CRLF or NUL bytes" if content.include?("\r") || content.include?("\0")
  errors << "#{path}: missing final newline" unless content.end_with?("\n")
  next if source_files.include?(path)
  content.each_line.with_index(1) do |line, number|
    errors << "#{path}:#{number}: trailing whitespace" if line.match?(/[\t ]+\n\z/)
  end
end
abort errors.join("\n") unless errors.empty?
puts "Format policy passed: #{source_files.length} C/header files (LF/NUL/final newline), #{text_files.length} infrastructure files (whitespace/newline)"
