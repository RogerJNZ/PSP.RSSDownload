====================================================================================
DownloadLinks - a file downloader and quick and dirty rss convertor for the Sony PSP 
====================================================================================

About DownloadLinks
===================
Downloadlinks allows you to download a list of files, of any type, using HTTP. 
It will also convert RSS files into HTML(ish) so that a standard web browser can be used 
to read them. An index page is created and all RSS and HTML files are linked to allow 
you to quickly read news from a large number of websites. 
			
So what's it for? I often travel overseas and have limited access to a wifi hotspot.
Generally there are free hotspots in cafes but it is often not convenient to try and 
surf all my favourite webpages while ignoring the company I am with (especially if they are paying ;-). Using download links I can quickly download my favourite rss feeds or even html pages for later reading at my leisure. 


Install
=======
To install simply copy DownloadLinks folder to your Memory Stick \PSP\GAME folder.

After you have run DownloadLinks run your favoutire web browser (e.g. Links2, Netfront 
Internet Browser by P86 or the standard PSP Browser) open /PSP/COMMON/INDEX.HTML amd 
add it as a bookmark for easy retrieval next time


Download list
=============
The list of links to download are stored in feeds.txt in the following format:
[name][space][url]
e.g. 
Slashdot.rss http://rss.slashdot.org/Slashdot/slashdot
Fark.rss http://www.fark.com/fark.rss
Engadget.rss http://www.engadget.com/rss.xml
Gizmodo.rss http://feeds.gawker.com/gizmodo/full
BoingBoing.html http://www.boingboing.net/
Wired.rss http://feeds.wired.com/wired/index?format=xml


Not that the name may not contain any spaces (or the url for that matter). In addtion, depending on which extension the filename contains one of the following additional 
processing will be performed
a) .html  a header and footer is added to the file and a link is added to
          /PSP/COMMON/index.html
b) .rss	  the xml file downloaded is converted into HTMLish (good enough to view in 
          your favourite browser) and a links is added to /PSP/COMMON/index.html
c) All other files are simply download and stored in /PSP/COMMON/


Configuration
=============
Config.ini allows you to override the following:
OUTPUT_DIR     - Override download directory
CONNECTTIMEOUT - Override timeout period for initial connection. Note that 
                 downloadlinks uses a single connection for all files to
                 reduce overheads
TIMEOUT
USERPWD	username:password f a username is required to access the internet
PROXY
PROXYPORT
PROXYUSERPWD
http://curl.haxx.se/libcurl/c/curl_easy_setopt.html For more details on these options.
Prepend the above connection names with CURLOPT_

Note: If config.ini does not exist then the DownloadLinks will use the following 
default values
OUTPUT_DIR=/PSP/COMMON/
CURLOPT_CONNECTTIMEOUT=5
CURLOPT_TIMEOUT=5
CURLOPT_USERPWD        [not used]
CURLOPT_PROXY          [not used]
CURLOPT_PROXYPORT      [not used]
CURLOPT_PROXYUSERPWD   [not used]


Acknowledgments
===============
Sakya - GUI was blatently stolen from Sakya's Homebrew Sorter. Big thanks to Sakya! 
(http://pspupdates.qj.net/PSP-homebrew-Homebrew-Sorter-Mod-GUI-B2/pg/49/aid/135257)

Modules
oslib 2.10          Oldschool library by Brunni. 
                    The original OSLib by Brunni can be found here:
                    http://brunni.palib.info/new/index.php?page=pspsoft_oslib
oslib_mod 1.0.1     http://www.sakya.it/OSLib_MOD/doc/html/
curl 7.16           http://curl.haxx.se/
PSPSDK              By ps2dev.org http://ps2dev.org/psp/Projects



=========
Changelog
=========


v4.0 - October 2009
============================
- Added GUI
- Update replace function
- Changed converted RSS layout

v3.0 - May 2009
============================
- Added configuration file
- Added error handler

v2.0 - March 2009
============================
- Added PSP network dialog

v1.0 - Febuary 2009
============================
- Initial version

