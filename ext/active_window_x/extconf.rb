require "mkmf"

map = {
  '1.8.' => 'RUBY_1_8',
  '1.9.' => 'RUBY_1_9',
}
version, micro = map.find do |v, m|
  RUBY_VERSION.start_with? v
end

if micro.nil?
then abort "Not supported ruby version: #{RUBY_VERSION}"
else $defs << "-D#{micro}"
end

if have_library('X11')
  create_makefile "active_window_x/xlib"
else
  abort "Cannot found X11"
end
