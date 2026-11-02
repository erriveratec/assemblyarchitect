require 'fileutils'
require 'pathname'

module TestAssetLinks
  ASSETS = %w[sound fonts data img].freeze

  def self.setup(output, root: File.expand_path('..', __dir__))
    root = Pathname.new(root).realpath
    output = Pathname.new(output).expand_path(root)
    relative = output.relative_path_from(root).each_filename.to_a
    raise ArgumentError, 'Test output must be below build/' unless relative.first == 'build' &&
      relative.length > 1 && !relative.include?('..')

    ancestor = root
    relative.each do |part|
      ancestor = ancestor.join(part)
      raise ArgumentError, "Symlinked output ancestor: #{ancestor}" if ancestor.symlink?
    end

    ASSETS.each do |name|
      asset = root.join(name)
      link = output.join(name)
      raise ArgumentError, "Missing repository asset: #{asset}" unless asset.directory?
      next unless link.exist? || link.symlink?
      raise ArgumentError, "Refusing non-symlink asset slot: #{link}" unless link.symlink?
      if link.exist? && link.realpath != asset.realpath
        raise ArgumentError, "Refusing link to another asset: #{link}"
      end
    end

    FileUtils.mkdir_p(output)
    ASSETS.each do |name|
      asset = root.join(name)
      link = output.join(name)
      next if link.exist?
      link.unlink if link.symlink?
      link.make_symlink(asset.relative_path_from(output))
    end
  end
end

if $PROGRAM_NAME == __FILE__
  abort 'Usage: ruby scripts/test_asset_links.rb OUTPUT_DIRECTORY' unless ARGV.length == 1
  TestAssetLinks.setup(ARGV.fetch(0))
end
