# ActiveWindowX

ActiveWindowX is a gem to observe an active window on Linux (X Window System).

## Installation

Add this line to your application's Gemfile:

	gem 'active_window_x'

And then execute:

	$ apt-get install libx11-dev
	$ bundle

Or install it yourself as:

	$ gem install active_window_x

## Usage

The following script observe the active window and it's title changing.

```simple.rb
require 'active_window_x'

ActiveWindowX::EventListener.new do |e|
  puts "#{e.type}:\t#{e.window}"
end
```

execute:

```
$ ruby simple.rb
 # switch the active window or the browser tab!!
active_window:	#<ActiveWindowX::Window:0x7fd47d391e18>
title:	#<ActiveWindowX::Window:0x7fd47d391850>
title:	#<ActiveWindowX::Window:0x7fd47d391030>
active_window:	#<ActiveWindowX::Window:0x7fd47d390540>
active_window:	#<ActiveWindowX::Window:0x7fd47d38fc30>
...
```

and see [other samples](https://github.com/kui/active_window_x/tree/master/sample)

## Contributing

1. Fork it
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Added some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create new Pull Request

## ToDo

* See `git grep TODO`
* Accessor API for the window icons
* Setter API to select event types for listening
