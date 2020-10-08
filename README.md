# Webserver

This is a simple multi-threaded webserver in C. Each request is executed in a separate thread.


Compile with 
```sh
gcc server.c
```

Run with
```sh
./a.out
```

Open your browser at 127.0.0.1:8080 or 127.0.0.1:8080/index.html to show the home page
Works with Google Chrome and Firefox in macOS, on linux only works on Firefox.
  
#### Sending files

You can send files with command
```sh
curl http://127.0.0.1:8080/ --upload-file samples/file.txt
```

and open the file at 127.0.0.1:8080/file.txt

By default the buffer size is fixed at 4096, you can increase this value if your file is bigger.
