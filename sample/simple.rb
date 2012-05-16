
require 'active_window_x'

ActiveWindowX::EventListener.new do |e|
  puts "#{e.type}:\t#{e.window}"
end
