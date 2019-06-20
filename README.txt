to run ftclient enter the following:
    python ftclient.py "hostname" "portnumber" "command" "filename"(optional) "dataport"
where "hostname" is where ftserver is currently running
      "portnumber" is the port on which the server will be listening
      "command" is either "-l" for directory listing or "-g" for file transfer
      "filename" is the name of the file to be transfered, including file extension, leave blank if using "-l" command
      "dataport" is the port number you want to run the data transfer on

compile ftserver first with the following command:
    make ftserver
to run ftserver enter the following:
    ftserver "portnumber"
where "portnumber" is the port you wish to have the server listen on
use "Ctrl+c" to stop the server running

