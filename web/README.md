# cardRack WEB interface

This is the web interface for the *cardRack* system. Copy all files and folders from the `server` folder to the root directory of a web server, which could be `localhost` (the default). You need to enable CGI processing in the directory `cgi-bin`. Most of the time, it should be enabled already. You also have to enable the execution bit for the CGI script in the `cgi-bin` folder.

You can now send a POST request to `localhost/storedata.cgi`. This can, for example, be accomplished with the `senddata.sh` script in the `test` folder. This script sends a password (see below) and the number of available cards (default 99).

The CGI script will then generate the file `newdata.js` in the `data` directory. When you now load the `index.html` from the web server root directory into a browser, it will show that there are 99 available cards. The webpage reloads every minute. So, when you change the number of available cards that will show up after at most 60 seconds. Waiting for three hours, one will get the message that the data is not valid anymore. 

The `conf` folder contains all the data necessary to set up a secure data connection. It is used by the `senddata.sh` script as well as by the Arduino sketch (by a soft link). It contains a simple password in `testkey.txt`. If you want to use a web server different from `localhost`, you can put that into `host.txt`. 

If you are going to use this interface, you should configure a secure password in `key.txt`. With a new password in `key.txt`, you also need a new hash in the file `data/key.sha` on the server. Once you have changed into the `conf` folder, the hash can be generated as follows:

```
echo -n $(cat key.txt) | shasum -a 256 > ../server/data/key.sha
```