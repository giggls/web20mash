#
# Regular cron jobs for the web20mash package
#
0 4	* * *	root	[ -x /usr/bin/web20mash_maintenance ] && /usr/bin/web20mash_maintenance
