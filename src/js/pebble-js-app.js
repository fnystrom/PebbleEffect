function appMessageAck(e) {
    console.log("options sent to Pebble successfully");
}

function appMessageNack(e) {
    console.log("options not sent to Pebble: " + e.error.message);
}

Pebble.addEventListener("showConfiguration", function() {
    var options = JSON.parse(window.localStorage.getItem('options'));
    var uri;
	
    if (options === null) {
        uri = 'http://effektprognos.azurewebsites.net/eliqeffekt.html';
    } else {
        uri = 'http://effektprognos.azurewebsites.net/eliqeffekt.html?' + 'accesstoken=' + encodeURIComponent(options.accesstoken)+ '&background=' + encodeURIComponent(options.background)+ '&temport=' + encodeURIComponent(options.temport)+ '&temperature=' + encodeURIComponent(options.temperature);
    }
    Pebble.openURL(uri);
});

Pebble.addEventListener("webviewclosed", function(e) {
    if (e.response !== '') {
        var options = JSON.parse(decodeURIComponent(e.response));
        window.localStorage.setItem('options', JSON.stringify(options));
		var watchoptions = {'background':options.background,'tempmode':options.temperature};
        //Pebble.showSimpleNotificationOnPebble("options", JSON.stringify(watchoptions));
		console.log(JSON.stringify(watchoptions));
		Pebble.sendAppMessage(watchoptions, appMessageAck, appMessageNack);
    } else {
        console.log("no options received");
    }
});

function today(){
  var _today = new Date();
  var dd = _today.getDate();
  var mm = _today.getMonth()+1; //January is 0!
  var yyyy = _today.getFullYear();
  if(dd<10){dd='0'+dd;} 
  if(mm<10){mm='0'+mm;} 
  _today = yyyy+'-'+mm+'-'+dd;
  return _today;
}

function getTemperature(){
  console.log("getTemperature");
  var response = "-1";
  var temport="";
  var options = JSON.parse(window.localStorage.getItem('options'));
  if (options === null) {
    Pebble.sendAppMessage({"temperature":"--\u00B0C"});
    return;
  } else {
    if (options.accesstoken === null){
      Pebble.sendAppMessage({"temperature":"--\u00B0C"});
      return;
    } else {
      temport = options.temport.toLowerCase();
    }
  }

  var request = new XMLHttpRequest();

  request.open('GET', "http://www.temperatur.nu/termo/"+temport+"/temp.txt", true);
	
  request.onreadystatechange = function(e) {
    if (request.readyState == 4) {
      if(request.status == 200 || request.status == 201) {
        response = parseFloat(request.responseText);
        console.log(response);
		Pebble.sendAppMessage({"temperature":response + "\u00B0C"});
      }
    }
  };	
  
  request.send(null);
	
  return response;
}

function fetchTodayEffect() {
  console.log("fetchTodayEffect");
  var response;
  var req = new XMLHttpRequest();
  var accesstoken="";
  var options = JSON.parse(window.localStorage.getItem('options'));
  if (options === null) {
    Pebble.sendAppMessage({"current":"--","forecast":"--"});
    return;
  } else {
    if (options.accesstoken === null){
      Pebble.sendAppMessage({"current":"--","forecast":"--"});
      return;
    } else {
      accesstoken = options.accesstoken;	
    }
  }

	req.open('GET', "https://my.eliq.se/api/data?accesstoken="+accesstoken+"&startdate="+today()+"&intervaltype=hour", true);
    req.onload = function(e) {
      if (req.readyState == 4) {
        if(req.status == 200 || req.status == 201) {
          response = JSON.parse(req.responseText);

          var current, estimate;
          if (response) {
			console.log(JSON.stringify(response));
            var hourCount = response.data.length;
            var totalEnergy=0;
			
            var i;
            for(i=0; i<hourCount;i++){
              var energy = response.data[i].energy;
              totalEnergy += energy;
            }
			
            current = Math.round(totalEnergy/1000) + "kWh";
            estimate = Math.round(totalEnergy/hourCount*24/1000) + "kWh";
                        
            Pebble.sendAppMessage({
              "current":current,
              "forecast":estimate});
          }
        } else {
          Pebble.sendAppMessage({
              "current":"--",
              "forecast":"--"});
          console.log("Error");
        }
      }
    };
	
  req.send(null);
}

function fetchCurrentEffect() {
  console.log("fetchCurrentEffect");
  var response;
  var req = new XMLHttpRequest();
  var accesstoken="";
  var options = JSON.parse(window.localStorage.getItem('options'));
  if (options === null) {
    Pebble.sendAppMessage({"current":"--","forecast":"--"});
    return;
  } else {
    if (options.accesstoken === null){
      Pebble.sendAppMessage({"current":"--","forecast":"--"});
      return;
    } else {
      accesstoken = options.accesstoken;	
    }
  }
    req.open('GET', "https://my.eliq.se/api/datanow?accesstoken=" + accesstoken, true);
    req.onload = function(e) {
      if (req.readyState == 4) {
        if(req.status == 200 || req.status == 201) {
          response = JSON.parse(req.responseText);

          var current;
          if (response) {
            console.log(JSON.stringify(response));

            current = response.power;
            
			Pebble.sendAppMessage({"justnu":current+"W"});
          }
        } else {
          Pebble.sendAppMessage({
              "justnu":"--"});
          console.log("Error");
        }
      }
    };
 
  req.send(null);
}

Pebble.addEventListener("ready",
                        function(e) {
                          //console.log("connect: " + e.type + ", ready:" + e.ready);
                          getTemperature();
                          fetchTodayEffect();
  						  fetchCurrentEffect();
							
                        });

Pebble.addEventListener("appmessage",
                        function(e) {
                          //console.log("message: " + JSON.stringify(e));
							if(e.payload.temperaturerequest == 1) {
								//console.log("TEMPERATURE REQUEST");
								getTemperature();
							} else if (e.payload.forecastpower == 1) {
								//console.log("FORECAST REQUEST");
								fetchTodayEffect();
							} else if (e.payload.currentpower == 1) {
								//console.log("CURRENT REQUEST");
								fetchCurrentEffect();
							}                    
                        });
