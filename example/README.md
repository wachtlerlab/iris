Example store files
===================

Store can be either at:
  - `/etc/iris/`
  - `~/.config/iris/`
  - `~/experiments/config`
  - `~/experiments/iris`


Store dir layout:

	/version
	/monitors/__ID__/__ID__.monitor (or __ID__.monitor)
	/__DATETIME__.settings
	/__DATETIME__.rgb2lms
	/default.monitor -> /monitors/__ID __ [symlink]
	/links.cfg
