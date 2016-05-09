// ======================================================================
//                ESP8266 create SPIFFS WEB server files script
// ======================================================================
// This file is not part of web server, it's just used as ESP8266 SPIFFS
// WEB server files preparation tool
// Please install dependencies with
// npm install zlib
// after all is installed just start by typing on command line
// node create_spiffs.js
// once all is fine, you can upload data tiles with Arduino IDE
//
// Written by Charles-Henri Hallard (http://hallard.me)
//
// ======================================================================

var uglify = require('uglify-js');
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
var stream = fs.createWriteStream(jsfile);
stream.once('open', function(fd) {

	console.log('Uglifying .js files');
	uglified = uglify.minify([ 
		"js/ajaxq.js", 
		"js/autofill.js", 
		"js/validator.js" 
	] 	); 

  stream.write(uglified.code);
  stream.end();

	console.log('Concataining already minified .js files');
	concat([
	  'js/jquery-2.1.4.min.js',
	  'js/bootstrap.min.js',
	  'js/bootstrap-table.min.js',
	  'js/bootstrap-table-fr-FR.min.js',
	  'js/bootstrap-notify.min.js',
	   jsfile
	], jsfile, function() {
							var gzip = zlib.createGzip();
							var inp = fs.createReadStream(jsfile);
							var out = fs.createWriteStream(gzjs);

							console.log('Compressing '+gzjs+' file');
							inp.pipe(gzip).pipe(out);
						  console.log('finished!');
						});
});


// =================
// CSS Files
// =================
console.log('Concataining already minified .css files');
concat([
  'css/bootstrap.min.css',
  'css/bootstrap-table.min.css',
  'css/remora.min.css'
], cssfile, function() {
						var gzip = zlib.createGzip();
						var inp = fs.createReadStream(cssfile);
						var out = fs.createWriteStream(gzcss);

						console.log('Compressing '+gzcss+' file');
						inp.pipe(gzip).pipe(out);
					  console.log('finished!');
					});


// =================
// index.html file
// =================
var gzip = zlib.createGzip();
var inp = fs.createReadStream(htmfile);
var out = fs.createWriteStream(gzhtm);

console.log('Compressing ' + gzhtm + ' file');
inp.pipe(gzip).pipe(out);
console.log('finished!');

