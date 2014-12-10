var URL = '';
var passw = '';
var psw = '';

function httpPost(url, params, cb) {
	var p = '';
	for (var param in params) {
		if (p !== '') {
			p += "&";
		}
		p += param + "=" + params[param];
	}
	var req = new XMLHttpRequest();
	req.open('POST', url, true);
	req.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
	req.setRequestHeader("Content-length", p.length);
	req.setRequestHeader("Connection", "close");

	req.onreadystatechange = function() {
		if(req.readyState == 4 && req.status == 200) {
			cb(req.responseText);
		}
	};
	req.send(p);
}

var pebbleSendQueue = {
	queue: [],
	queueFull: false,
	send: function(msg) {
		if (this.queueFull) {
			this.queue.push(msg);
			return;
		}
		this.queueFull = true;
		this._doSend(msg);
	},
	_sendDone: function(e) {
		if (this.queue.length === 0) {
			this.queueFull = false;
			return;
		}
		var msg = pebbleSendQueue.queue.splice(0,1)[0];
		this._doSend(msg);
	},
	_sendFailed: function(e) {
		var msg = pebbleSendQueue.queue.splice(0,1)[0];
		this._doSend(msg);
	},
	_doSend: function(msg) {
		this.inQueue = msg;
		Pebble.sendAppMessage(msg, function(e) { pebbleSendQueue._sendDone(e); }, function(e) { pebbleSendQueue._sendFailed(e); });
	}
};
 
function parseHtmlEnteties(str) {
    return str.replace(/&#([0-9]{1,3});/gi, function(match, numStr) {
        var num = parseInt(numStr, 10); // read num as normal number
        return String.fromCharCode(num);
    });
}

var devices = {};
var options = {};
var events = {};

function fetchDevicesNexaHome( ) {
	var req = new XMLHttpRequest();
	var urlen = URL+'/nexahome?json=yes';
	urlen = urlen + passw;
	req.open('GET',encodeURI(urlen));
	req.onreadystatechange = function() {
		if (req.readyState == 4) {
      // 200 - HTTP OK
      if(req.status == 200) {
				var decodetext = parseHtmlEnteties(req.responseText);
				// adjust error for json format
				decodetext = decodetext.replace(/\n|\r/g, "");
				decodetext = decodetext.replace('"devices": {','"devices": [');
				decodetext = decodetext.replace('},    "events":','],    "events":');
						
				var jsons = JSON.parse(decodetext);

				pebbleSendQueue.send({ "module": "clear" });
				
				var current = 0;
				options = jsons.status.mode.options;
				
				for (var j in options) {
					if (jsons.status.mode.current == options[j]) current = j;
		
					pebbleSendQueue.send({
						module:		'status',
						name:			options[j],
						state:		parseInt(current),
						id:				parseInt(j),
					});
				}
							
				var name;
				
				events = jsons.events;
				if(localStorage.getItem('nexahome_events') == 'true') {			
					for (j in events) {					
						name = events[j].name;					
						if (name.length > 16) {
							name = name.substr(0, 16);
						} 

						pebbleSendQueue.send({
							module:		'event',
							name:			name,
							temp:			events[j].time +" " + events[j].cmd,
						});
					}
				}

				devices = jsons.devices;
				for (var i in devices) {
				
					name = devices[i].name;
					
					var evntd = "No Event";
						for (j in events) {			
							if (events[j].name == name) {
								evntd = events[j].time + " " + events[j].cmd;
								evntd = evntd.replace(/-/g, "");
								evntd = evntd.replace(/20/,"");
								break;
							}
						}
					var module = "device";
					
					if (name.length > 16) {
						name = name.substr(0, 16);
					} 
					
					var state = 2;
					if (devices[i].status) {
						if (devices[i].status == 'ON') {
							state = 1;
						}
					}

					var temp ='0';
					var humidity = '';
					if('sensor' in devices[i]) {
						module = "sensor";
						if (devices[i].sensor !== "") 
							temp = devices[i].sensor;
						var idd = localStorage.getItem(devices[i].id);
						if (idd != '0') {
							for (var ii in devices) {
								if (idd == devices[ii].id) {
									humidity = devices[ii].sensor;
									break;
								}							
							}
						}						
						else
							devices[i].sensor = temp;					
					}

					var methods = 3;
						if (devices[i].dimmable === true) {
							methods = 16;
						}

					var level = '0';
					if (devices[i].level)
						level = devices[i].level;
					else {
						devices[i].level = level;
					}
					
					if (module == "device") {
						temp = evntd;
					}
					if((localStorage.getItem('nexahome_devices') == 'true' && module == "device") || 
						(localStorage.getItem('nexahome_sensors') == 'true' && module == "sensor")) {			
						if (localStorage.getItem(devices[i].name) == 'true' || !localStorage.getItem(devices[i].name)) {

							pebbleSendQueue.send({
								module:		module,
								name:			name,
								state:		parseInt(state),
								dimvalue: parseInt(level),
								id:				parseInt(devices[i].id),
								temp:			temp,
								methods:	parseInt(methods),
								timestamp: devices[i].timestamp,
								hum:			humidity
							});
						}
					}				
				}				
				pebbleSendQueue.send({ "module": "done" });				
      } else {
				Pebble.showSimpleNotificationOnPebble("Error","Request returned error code " + req.status.toString());				
      }
    }
  };
  req.send(null);
}

// Set callback for the app ready event
Pebble.addEventListener("ready", function(e) {
	
	if (localStorage.getItem('passw') !== '') {
		psw = localStorage.getItem('passw');
		passw = '&psw=' + psw;
	}

	if (localStorage.getItem('URL') !== '') {
		URL = localStorage.getItem('URL');
	}
	
	if (URL !== null)
		fetchDevicesNexaHome( );
});

Pebble.addEventListener("appmessage", function(e) {
	var msg = e.payload;
	var id = msg.id;
	var param;
	var state;

	if (!('module' in msg)) {
		return;
	}

	if (msg.module == 'status') {
		
		param = passw + '&bsh=mode("' + options[id] +'")';
		httpPost(encodeURI(URL+'/nexahome'+ param), '', function(r) {
		});
		
	} else if (msg.module == 'device') {
		
		// find device with id
		for (var i in devices) {
			if (parseInt(devices[i].id) == id) {
				break;
			}
		}

		if (msg.methods == 3) {
			if (devices[i].status == 'ON') {
				state = 2;
				param = passw + '&bsh=deviceOff("' + id + '");';
				if (devices[i].dimmable === true) 
					devices[i].level = 0;
			} else {
				state = 1;
				param = passw +  '&bsh=deviceOn("' + id + '");';
				if (devices[i].dimmable === true) {
					devices[i].level = 100;
					param = passw + '&bsh=deviceLevel("' + id + '","' + devices[i].level + '%")';
				}
			}
		}

		if (msg.methods == 16) {
			devices[i].level = msg.dimvalue.toString();
			param = passw + '&bsh=deviceLevel("' + id + '","' + devices[i].level + '%")';
			state = 1;
			if(parseInt(devices[i].level) === 0) {
				state = 2;
				param = passw + '&bsh=deviceOff("' + id + '");';
			}
		}

		httpPost(encodeURI(URL+'/nexahome'+ param), '', function(r) {
			if (state == 2)
				devices[i].status ='OFF';
			else
				devices[i].status ='ON';
		});

		var methods = 3;
		if (devices[i].dimmable === true) {
			methods = 16;
		}

		pebbleSendQueue.send({
			module: "device",
			name: devices[i].name,
			state: parseInt(state),
			methods: parseInt(methods),
			dimvalue: parseInt(devices[i].level),
			temp:	"",
			id: parseInt(id),
			timestamp: devices[i].timestamp
		});

	}
});

Pebble.addEventListener("showConfiguration", function() {
	var confurl = 'https://dl.dropboxusercontent.com/u/29205101/cn112.html';
	confurl = confurl + '?URL=';
	if (localStorage.getItem('URL') !== '') {
				confurl = confurl + localStorage.getItem('URL');
	}
	
	var passw =  localStorage.getItem('passw') ? localStorage.getItem('passw') : "";
	confurl = confurl + '&passw=' + passw;
	
	var stat =  localStorage.getItem('nexahome_sensors') ? localStorage.getItem('nexahome_sensors') : 'true';
	confurl = confurl + "&nexahome_sensors=" + stat;
	
	stat =  localStorage.getItem('nexahome_devices') ? localStorage.getItem('nexahome_devices') : 'true';
	confurl = confurl + "&nexahome_devices=" + stat;
	
	stat =  localStorage.getItem('nexahome_events') ? localStorage.getItem('nexahome_events') : 'true';
	confurl = confurl + "&nexahome_events=" + stat;

	confurl = confurl + "&" + encodeURIComponent("Select devices to include in watch app:") + "=cblabel";
	for(var i in devices) {
		stat =  localStorage.getItem(devices[i].name) ? localStorage.getItem(devices[i].name) : 'true';
		confurl = confurl + '&' + encodeURIComponent(devices[i].name) +'=' + encodeURIComponent(stat);
		if ('sensor' in devices[i]) {
			var id =  localStorage.getItem(devices[i].id) ? localStorage.getItem(devices[i].id) : '0';
			confurl = confurl + '&' + encodeURIComponent(devices[i].id) +'=' + encodeURIComponent(id);
		}
	}
	Pebble.openURL(confurl);
});

function empty(o) {
	for(var i in o) 
		if(o.hasOwnProperty(i))
			return false;
	return true;
}

Pebble.addEventListener("webviewclosed", function(e) {
  // webview close	
	// adjust ios return to many characters for åöä
	var options = encodeURIComponent(e.response);
  options = options.replace(/%83%C2/g,"");
  options = JSON.parse(decodeURIComponent(options));
	if (empty(options) === false) {
		localStorage.clear();
	}
	for ( var i in options )
	{
		if ( options.hasOwnProperty( i ) )
		{		
			for (var j in devices) {
				if (i == devices[j].name) {
					break;
				}
			}			
			localStorage.setItem(i,  options[i]);
		}
	}
	
	if(!options.URL) {
		localStorage.setItem('URL', URL);		
	}	
	
	if(options.hasOwnProperty( 'passw'))
		psw = options.passw;
	localStorage.setItem('passw', psw);
	
});


 