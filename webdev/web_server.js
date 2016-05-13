// ======================================================================
//                      ESP8266 WEB Server Simulator
// ======================================================================
// This file is not part of web server, it's just used as ESP8266 
// Simulator to check HTLM / JQuery and all web stuff without needing 
// to flash ESP8266 target, you'll need nodejs to run it
// Please install dependencies with
// npm install mime httpdispatcher websocket mime formidable
// after all is installed just start by typing on command line
// node web_server.js
// once all is fine, just run the script create_spiffs.js to publish
// files to data folder, then you can upload data tiles with Arduino IDE
// ======================================================================

var http = require('http');
var fs = require('fs');
var path = require('path');
var url = require('url');
var mime = require('mime');
var formidable = require("formidable");
var util = require('util');
var os = require('os');
var dispatcher = require('httpdispatcher');
var interval;
var startTime = Date.now();
var ws = require('websocket').server;
var temperature=20;
var humidity=50;


var config = {
"ssid":"CH2I-HOTSPOT",
"psk":"DummyOne",
"host":"NRJMeter_115F0B",
"appsk":"",
"apssid":"OTA_NRJMeter",
"ip":"0.0.0.0",
"mask":"0.0.0.0",
"gw":"0.0.0.0",
"dns":"0.0.0.0",
"emon_host":"emoncms.org",
"emon_port":80,
"emon_url":"/input/post.json",
"emon_apikey":"",
"emon_node":0,
"emon_freq":0,
"jdom_host":"",
"jdom_port":80,
"jdom_url":"",
"jdom_apikey":"",
"jdom_adco":"",
"jdom_freq":0,
"domz_host":"domoticz.local",
"domz_port":80,
"domz_url":"/json.htm?type=command&param=udevice&nvalue=0",
"domz_user":"",
"domz_pass":"",
"domz_idx":0,
"domz_freq":0,
"cnt_io_1":255,
"cnt_dly_1":10,
"cnt_val_1":0,
"cnt_io_2":255,
"cnt_dly_2":10,
"cnt_val_2":0,
"ota_auth":"",
"ota_port":8266,
"sens_si7021":0,
"sens_sht10":0,
"sens_hum_led":"29,66",
"sens_temp_led":"8,58",
"cfg_rgb":1,
"cfg_debug":1,
"cfg_oled":0,
"cfg_ap":0,
"cfg_wifi":1,
"cfg_static":0,
"cfg_led_bright":50,
"cfg_led_hb":1,
"sens_freq":300
}

var fp ={
	"fp1": "A",
	"fp2": "E",
	"fp3": "E",
	"fp4": "E",
	"fp5": "C",
	"fp6": "H",
	"fp7": "C"
}

var spiffs = { 
	"files":[
	{"na":"/css/nrjmeter.css.gz","va":"24325"}
	,{"na":"/favicon.ico","va":"1150"}
	,{"na":"/fonts/glyphicons.woff","va":"23424"}
	,{"na":"/fonts/glyphicons.woff2","va":"18028"}
	,{"na":"/index.htm.gz","va":"6026"}
	,{"na":"/js/main.js.gz","va":"6131"}
	,{"na":"/js/nrjmeter.js.gz","va":"80913"}
	,{"na":"/version.json","va":"57"}
	],
	"spiffs":[
	{"Total":957314, "Used":163652, "ram":21440}
	]
}

var wifiscan = [
	{"ssid":"FreeWifi_secure","rssi":-59,"enc":"????","chan":1},
	{"ssid":"HOME-FREEBOX","rssi":-60,"enc":"WPA2","chan":1},
	{"ssid":"FreeWifi","rssi":-60,"enc":"Open","chan":1},
	{"ssid":"CH2I-HOTSPOT","rssi":-61,"enc":"WPA2","chan":9},
	{"ssid":"HP-Print-3D-Deskjet 3520 series","rssi":-77,"enc":"Open","chan":6},
	{"ssid":"Livebox-0479","rssi":-93,"enc":"Auto","chan":6},
	{"ssid":"HOME-HOTSPOT","rssi":-60,"enc":"WPA2","chan":9}
	]


function system() {
return[
	{"na":"Uptime","va":((Date.now()-startTime)/1000).toFixed(0)},
	{"na":"Firmware Version","va":"1.0"},
	{"na":"Compile le","va":"Feb 28 2016 04:35:12"},
	{"na":"SDK Version","va":"1.5.1(e67da894)"},
	{"na":"Chip ID","va":"0x115F0B"},
	{"na":"Boot Version","va":"0x1F"},
	{"na":"Reset cause","va":"External System"},
	{"na":"Flash Real Size","va":"4.00 MB"},
	{"na":"Flash IDE Speed","va":"40MHz"},
	{"na":"Flash IDE Mode","va":"QIO"},
	{"na":"Firmware Size","va":"373.69 KB"},
	{"na":"Free Size","va":"2.63 MB"},
	{"na":"Analog","va":"6 mV"},
	{"na":"Wifi Mode","va":"STA"},
	{"na":"Wifi Mode","va":"N"},
	{"na":"Wifi Channel","va":"9"},
	{"na":"Wifi AP ID","va":"0"},
	{"na":"Wifi Status","va":"5"},
	{"na":"Wifi Autoconnect","va":"1"},
	{"na":"SPIFFS Total","va":"934.88 KB"},
	{"na":"SPIFFS Used","va":"159.82 KB"},
	{"na":"SPIFFS Occupation","va":"17%"},
	{"na":"Free Ram","va":humanSize(os.freemem())}
	]
}


function humanSize(bytes) {
	var units =  ['kB','MB','GB','TB','PB','EB','ZB','YB']
	var thresh = 1024;
	if(Math.abs(bytes) < thresh) 
		return bytes + ' B';

	var u = -1;
	do {
		bytes /= thresh;
		++u;
	} 
	while(Math.abs(bytes) >= thresh && u < units.length - 1);
	return bytes.toFixed(1)+' '+units[u];
}

//Lets use our dispatcher
function handleRequest(req, res) {
  try {
    console.log(req.url);
    dispatcher.dispatch(req, res);
  } 
  catch(err) {
    console.log(err);
  }
}

// This should work both there and elsewhere.
function isEmptyObject(obj) {
  for (var key in obj) {
    if (Object.prototype.hasOwnProperty.call(obj, key)) {
      return false;
    }
  }
  return true;
}

dispatcher.onError(function(req, res) {
  var uri = url.parse(req.url).pathname;
	var filePath = '.' + uri;
	var extname = path.extname(filePath);
	var url_parts = url.parse(req.url, true);
	var query = url_parts.query;
	var contentType ;

 	//console.log(util.inspect({query: query}));

		// Check first Query posted http://ip/?toto=titi
		if (!isEmptyObject(query)) {

		 	if (query.fp != undefined && query.fp.length==7){
		 		console.log("FP="+query.fp);
		 		for (var i=1; i<=7; i++) {
		 			fp["fp"+i] = query.fp.charAt(i-1);
		 		}
		 		console.log( util.inspect({fp: fp}));
			  res.writeHead(200, {"Content-Type": "text/json"});
			  res.end('{"response":0}');

		 	} else if  (query.setfp != undefined && query.setfp.length==2) {
		 		console.log("setfp="+query.setfp);
		 		var i = query.setfp.charAt(0);
		 		var o = query.setfp.charAt(1).toUpperCase();

			  res.writeHead(200, {"Content-Type": "text/json"});

		 		if (i>='1' && i<='7' && (o=='C'||o=='A'||o=='E'||o=='H'||o=='1'||o=='2') ) {
		 			fp["fp"+i] = o;
			  	res.end('{"response":0}');
			 		console.log( util.inspect({fp: fp}));
		 		} else {
				  res.end('{"response":1}');
				}

		 	} else {
        res.writeHead(500);
        res.end('Sorry, unknown or bad query received: '+query+' ..\n');
		 	}

		// serve Web page
		} else {

			if (filePath == './') {
  			filePath = './index.htm';
			}
			
			contentType = mime.lookup(filePath);

			// Stream out he file
			fs.readFile(filePath, function(error, content) {
		    if (error) {
		      if(error.code == 'ENOENT'){
		        fs.readFile('./404.html', function(error, content) {
		          res.writeHead(200, { 'Content-Type': contentType });
		          res.end(content, 'utf-8');
							console.log("ENOENT "+filePath+ ' => '+contentType);
		        });
		      }
		      else {
		        res.writeHead(500);
		        res.end('Sorry, check with the site admin for error: '+error.code+' ..\n');
		        res.end(); 
						console.log("Error "+filePath+ ' => '+contentType);
		      }
		    }
		    else {
		      res.writeHead(200, { 'Content-Type': contentType });
		      res.end(content, 'utf-8');
					console.log("Sent "+filePath+ ' => '+contentType);
		    }
		  });
		}


});


function rnd(low, high) {
	return Math.floor((Math.random() * 100 ) * (high - low) + low) ;
}

function rTemp() { 
	temperature = (rnd(-20,20) + 20) / 100.0; 
	return temperature;
}

function rHum() { 
	humidity = (rnd(-20,20) + 50)/100.0; 
 return humidity;
}

function sensors() {  
	var sensors =	{	"si7021":[ {"temperature":rTemp(),	"humidity":rHum(),	"seen":1}	],
									 "sht10":[ {"temperature":rTemp(),	 "humidity":rHum(),	"seen":1}	]	}
	return sensors;
}

function log(con, msg) {
	console.log(msg);
	con.sendUTF(JSON.stringify({message:"log", data:msg}));
}


dispatcher.onGet("/sensors", function(req, res) {
      res.writeHead(200, {"Content-Type": "text/json"});
      res.end(JSON.stringify(sensors()));
});    

dispatcher.onGet("/system", function(req, res) {
			//console.log('s[0]=' + util.inspect(system[0], false, null));
			//system[0].va = ((Date.now()-startTime)/1000).toFixed(0);
			//system[1].va = humanSize(os.freemem());
      res.writeHead(200, {"Content-Type": "text/json"});
      res.end(JSON.stringify(system()));
});    

dispatcher.onGet("/spiffs", function(req, res) {
      res.writeHead(200, {"Content-Type": "text/json"});
      res.end(JSON.stringify(spiffs));
});    

dispatcher.onGet("/config", function(req, res) {
      res.writeHead(200, {"Content-Type": "text/json"});
      res.end(JSON.stringify(config));
});  

dispatcher.onGet("/fp", function(req, res) {
      res.writeHead(200, {"Content-Type": "text/json"});
      res.end(JSON.stringify(fp));
});  

dispatcher.onGet("/?", function(req, res) {
			//Store the data from the fields in your data store.
			//The data store could be a file or database or any other store based
			//on your application.
			var fields = [];
			var form = new formidable.IncomingForm();
			form.on('field', function (field, value) {
			    console.log(field);
			    console.log(value);
			    fields[field] = value;
			});

			form.on('end', function () {
			  res.writeHead(200, {"Content-Type": "text/json"});
 			  res.end('{"response":0}');
			});
			form.parse(req);

}); 

dispatcher.onGet("/wifiscan.json", function(req, res) {
			setTimeout(function() {
	      						res.writeHead(200, {"Content-Type": "text/json"});
  	    						res.end(JSON.stringify(wifiscan));
									}, 1000, req, res);
}); 

dispatcher.onGet("/hb", function(req, res) {
      res.writeHead(200, {"Content-Type": "text/html"});
      res.end("OK");
});    

var server = http.createServer(handleRequest);
var wsSrv = new ws({ httpServer: server });

wsSrv.on('request', function(request) {
	var connection = request.accept('', request.origin);
	console.log("+++ Websocket client connected!");
	clearInterval(interval);

	connection.on('message', function(message) {
		console.log('WS ' + util.inspect(message, false, null));

		if (message.type === 'utf8') {
			var msg = message.utf8Data.split(':');
			var value = msg[1];
			msg = msg[0]
			console.log('WS  msg="' + msg + '" value="'+value+'"');
			// Command message
			if ( msg.charAt(0)=='$' ) 
			{
				clearInterval(interval);

				if (msg==='$system') {
					interval = setInterval(function(){connection.sendUTF(JSON.stringify({message:"system", data:system()}));}, 1000);
					connection.sendUTF(JSON.stringify({message:"system", data:system()}));
					//connection.sendUTF('\'{message:"sensors", data:[{"na":"Uptime","va":"17"},{"na":"Board Version","va":"1.0.0"},{"na":"Compile le","va":"Jan 20 2016 18:54:14"}]}\'');
				} else if (msg==='$sensors') {
					interval = setInterval(function(){connection.sendUTF(JSON.stringify({message:"sensors", data:sensors()}));},value*1000);
					connection.sendUTF(JSON.stringify({message:"sensors", data:sensors()}));
				}
			} else {
				connection.sendUTF("Reveived your raw message '" + msg + "'");

				//log(connection, "Reveived your raw message '" + msg + "'");
			}

		}
		else if (message.type === 'binary') {
			console.log('Received Binary Message of ' + message.binaryData.length + ' bytes');
			connection.sendBytes(message.binaryData);
		}
	});

	connection.on('close', function(reasonCode, description) {
		console.log((new Date()) + ' Peer ' + connection.remoteAddress + ' disconnected.');
	});
});

//Lets start our server
server.listen(8088, function() {
  //Callback triggered when server is successfully listening. Hurray!
  console.log("Server listening on: http://localhost:%s", 8088);
});