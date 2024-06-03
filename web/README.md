# cardRack WEB interface

This is the web interface for the *cardRack* system. Before you can start, you need access to a web server. The name of this server needs to be stored in the file `config/example.conf`, replacing the name `example.org`.  In doing so, you may also want to replace the default password `examplepassword` in the second line. After that, change into `config` folder, and the start the shell script `genlocation.sh` with the argument `example`  in order to generate a few configuration files in the web server folder and the Arduino sketch folder. 

Now, copy all files and folders from the `server` folder to the root directory of a web server. You need to enable CGI processing in the directory `cgi-bin`. Most of the time, it should be enabled already. You also have to set the *execution enable bit* for the CGI script in the `cgi-bin` folder.

You can now send a POST request to `<your-server>/cgi-bin/storedata.cgi`. This can, for example, be accomplished with the `senddata.sh` script in the `test` folder. This script sends a location (passed as the first argument, which should be `example` when you first try it out), a password (see above) and the number of available cards (passed as the second argument).

The CGI script will then generate the file `example.txt` in the `data` directory. When you now load the `example.php` page from the web server root directory into a browser, it will show you the number of  available cards.  Waiting for two hours and a few minutes, one will get the message that the data is not valid anymore. 

If you plan to use WordPress to display the number of cards, you can use the code snippet provided in `codesnippet.php`. This needs to be activated by, e.g., using the [CodeSnippet](https://de.wordpress.org/plugins/code-snippets/) plugin. 

If you want to create other configurations for other locations, simply generate a new file `<location-name>.conf` with the server name in the first line and the password in the second line and run the `genlocation.sh` script again.  You need to make the appropriate changes in the Arduino sketch as well. 
