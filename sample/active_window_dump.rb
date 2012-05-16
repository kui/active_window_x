# dump all properties of window on changing active window

require 'rubygems'
require 'active_window_x'

@listener = ActiveWindowX::EventListener.new

# when pressing Ctrl-C
trap :INT do
  @listener.destroy
  exit true
end

@listener.start do |e|
  next until e.window

  w = e.window
  puts <<__OUTPUT__ if e.type == :active_window
######### change active window  #########
     id:	#{w.id}
     title:	#{w.title}
     name:	#{w.app_name}
     class:	#{w.app_class}
     pid:	#{w.pid}
     command:	#{w.command}
__OUTPUT__
  puts <<__OUTPUT__ if e.type == :title
######### change title #########
     title:	#{w.title}
__OUTPUT__
end
