
require 'active_window_x'

ActiveWindowX::EventListener.new do |e|
  puts(e.window ?
       "#{e.type}:\t#{e.window.app_name}\t#{e.window.title}" :
       "#{e.type}")
end
