#!/usr/bin/env rake

require "bundler/gem_tasks"
require "rspec/core"
require "rspec/core/rake_task"
require 'rake/clean'

NAME = 'active_window_x'
MAKEFILES = Dir.glob "ext/**/Makefile"

## so file
file "lib/#{NAME}/xlib.so" =>
  (Dir.glob("ext/#{NAME}/*.{rb,c}") << "lib/#{NAME}")do

  Dir.chdir "ext/#{NAME}" do
    ruby "extconf.rb"
    sh "make"
  end

  cp "ext/#{NAME}/xlib.so", "lib/#{NAME}"
end

## make_clean
desc "do `make clean` with all Makefiles"
task :make_clean do
  MAKEFILES.each do |file|
    dir = File.dirname file
    puts "cd #{dir}"
    Dir.chdir dir do
      sh "make clean"
    end
  end
end

## clobber
CLOBBER.include "lib/**/*.so"

## clean
CLEAN.include MAKEFILES
task :clean => :make_clean

desc "Run all specs in spec/*_spec.rb"
task :spec => "lib/#{NAME}/xlib.so"
RSpec::Core::RakeTask.new :spec

task :build => :spec
task :default => :spec
