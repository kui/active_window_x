# -*- coding:utf-8; mode:ruby; -*-
require File.expand_path('../lib/active_window_x/version', __FILE__)

Gem::Specification.new do |gem|
  gem.authors       = ["Keiichiro Ui"]
  gem.email         = ["keiichiro.ui@gmail.com"]
  gem.description   = %q{TODO: Write a gem description}
  gem.summary       = %q{TODO: Write a gem summary}
  gem.homepage      = ""

  gem.files         = `git ls-files`.split($\)
  gem.extensions    = ['ext/active_window_x/extconf.rb']
  gem.executables   = gem.files.grep(%r{^bin/}).map{ |f| File.basename(f) }
  gem.test_files    = gem.files.grep(%r{^(test|spec|features)/})
  gem.name          = "active_window_x"
  gem.require_paths = ["lib"]
  gem.version       = ActiveWindowX::VERSION

  gem.add_development_dependency 'rake'
  gem.add_development_dependency 'bundler'
  gem.add_development_dependency 'rspec'
end
