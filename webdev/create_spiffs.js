// ======================================================================
//                ESP8266 create SPIFFS WEB server files script
// ======================================================================
// This file is not part of web server, it's just used as ESP8266 SPIFFS
// WEB server files preparation tool
// Please install dependencies with
// npm install
// after all is installed just start by typing on command line
// npm run create_spiffs
// once all is fine, you can upload data tiles with Arduino IDE
//
// Written by Charles-Henri Hallard (http://hallard.me)
//
// ======================================================================

var compressor = require('node-minify');
//var uglify = require('uglify-js');
var concat = require('concat-files');
var zlib = require('zlib');
var fs = require('fs');


var jsfile = "js/remora.js";
var cssfile = "css/remora.css";
var htmfile = "index.htm";


var gzjs  = "../data/" + jsfile  + ".gz";
var gzcss = "../data/" + cssfile + ".gz";
var gzhtm = "../data/" + htmfile + ".gz";

// =================
// javascript Files
// =================
console.log('Uglifying .js files');
compressor.minify({
  compressor: 'yui-js',
  input: [
    "js/ajaxq.js",
    "js/autofill.js",
    "js/validator.js",
    'js/main.js'
  ],
  output: jsfile,
  sync: true,
  callback: function(err, value) {
    if (err) {
      console.log("Error minify JS: ", err);
      return;
    }
    console.log('Concataining already minified .js files');
    concat([
      'js/jquery-2.1.4.min.js',
      'js/bootstrap.min.js',
      'js/bootstrap-table.min.js',
      'js/bootstrap-table-fr-FR.min.js',
      'js/bootstrap-notify.min.js',
      'js/bootstrap-slider.min.js',
       jsfile
    ], jsfile, function() {
                var gzip = zlib.createGzip();
                var inp = fs.createReadStream(jsfile);
                var out = fs.createWriteStream(gzjs);

                console.log('Compressing '+gzjs+' file');
                inp.pipe(gzip).pipe(out);
                console.log('finished!');
              });
  }
});

// =================
// CSS Files
// =================
compressor.minify({
  compressor: 'yui',
  input: 'css/main.css',
  output: cssfile,
  callback: function(err, min) {
    console.log('Concataining already minified .css files');
    concat([
      'css/bootstrap.min.css',
      'css/bootstrap-table.min.css',
      'css/bootstrap-slider.min.css',
      cssfile
    ], cssfile, function() {
          var gzip = zlib.createGzip();
          var inp = fs.createReadStream(cssfile);
          var out = fs.createWriteStream(gzcss);

          console.log('Compressing '+gzcss+' file');
          inp.pipe(gzip).pipe(out);
          console.log('finished!');
        });
  }
});


// =================
// index.html file
// =================
var gzip = zlib.createGzip();
var inp = fs.createReadStream(htmfile);
var out = fs.createWriteStream(gzhtm);

console.log('Compressing ' + gzhtm + ' file');
inp.pipe(gzip).pipe(out);

// =================
// fonts
// =================
var fonts = ["glyphicons.woff", "glyphicons.woff2", "jeedom2.woff"];
for (var i = 0; i < fonts.length; i++) {
  console.log('Copy font: ' + fonts[i]);
  fs.createReadStream('fonts/' + fonts[i]).pipe(fs.createWriteStream('../data/fonts/' + fonts[i]));
}
console.log('finished!');
