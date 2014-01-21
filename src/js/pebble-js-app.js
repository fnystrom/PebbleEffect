function appMessageAck(e) {
    //console.log("options sent to Pebble successfully");
}

function appMessageNack(e) {
    //console.log("options not sent to Pebble: " + e.error.message);
}

Pebble.addEventListener("showConfiguration", function() {
    var options = JSON.parse(window.localStorage.getItem('options'));
    var uri;
	
    if (options === null) {
        uri = 'http://effektprognos.azurewebsites.net/eliqeffekt.html';
    } else {
        uri = 'http://effektprognos.azurewebsites.net/eliqeffekt.html?' + 'accesstoken=' + encodeURIComponent(options.accesstoken)+ '&background=' + encodeURIComponent(options.background)+ '&mode=' + encodeURIComponent(options.mode);
    }
    Pebble.openURL(uri);
});

Pebble.addEventListener("webviewclosed", function(e) {
    if (e.response !== '') {
        var options = JSON.parse(decodeURIComponent(e.response));
        window.localStorage.setItem('options', JSON.stringify(options));
		var background = {'background':options.background,'mode':options.mode};
        
		Pebble.sendAppMessage(background, appMessageAck, appMessageNack);
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
  var response = "-1";

  var request = new XMLHttpRequest();

  request.open('GET', "http://www.temperatur.nu/termo/toltorpsdalen/temp.txt", true);
	
  request.ontimeout = function(f) {
    console.log("timeout");
  }
  request.onerror = function(f) {
    console.log("error");
  }
	
  request.onreadystatechange = function(e) {
    if (request.readyState == 4) {
      if(request.status == 200 || request.status == 201) {
		console.log();
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
  var response;
  var req = new XMLHttpRequest();
  var accesstoken="";
//  var mode="";
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
//      mode = options.mode;
    }
  }
  
  //console.log("storing options: " + JSON.stringify(options));
	
//  if(mode == "forecast"){
    req.open('GET', "https://my.eliq.se/api/data?accesstoken="+accesstoken+"&startdate="+today()+"&intervaltype=hour", true);
    //console.log("Calling eliq data...");
    req.onload = function(e) {
      if (req.readyState == 4) {
        if(req.status == 200 || req.status == 201) {
          response = JSON.parse(req.responseText);

          var current, estimate;
          if (response) {
            var hourCount = response.data.length;
            var totalEnergy=0;
			
            var i;
            for(i=0; i<hourCount;i++){
              var energy = response.data[i].energy;
              totalEnergy += energy;
            }
			
            current = Math.round(totalEnergy/1000) + "kWh";
            estimate = Math.round(totalEnergy/hourCount*24/1000) + "kWh";
                        
            //console.log("current=" + current + ", estimate=" + estimate);
			
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
  /*} else {
    req.open('GET', "https://my.eliq.se/api/datanow?accesstoken=" + accesstoken, true);
    //console.log("Calling eliq datanow...");
    req.onload = function(e) {
      if (req.readyState == 4) {
        if(req.status == 200 || req.status == 201) {
          response = JSON.parse(req.responseText);

          var current;
          if (response) {
            console.log(JSON.stringify(response));

            current = response.power;
            var diff = 0;

            Pebble.sendAppMessage({
              "current":current+"W",
              "forecast":diff+"W"});
          }
        } else {
          Pebble.sendAppMessage({
              "current":"--",
              "forecast":"--"});
          console.log("Error");
        }
      }
    };
  }*/

  req.send(null);
}

function fetchCurrentEffect() {
  var response;
  var req = new XMLHttpRequest();
  var accesstoken="";
  //var mode="";
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
      //mode = options.mode;
    }
  }
    req.open('GET', "https://my.eliq.se/api/datanow?accesstoken=" + accesstoken, true);
    //console.log("Calling eliq datanow...");
    req.onload = function(e) {
      if (req.readyState == 4) {
        if(req.status == 200 || req.status == 201) {
          response = JSON.parse(req.responseText);

          var current;
          if (response) {
            console.log(JSON.stringify(response));

            current = response.power;
            var diff = 0;

            Pebble.sendAppMessage({
              "current":current+"W",
              "forecast":diff+"W"});
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

Pebble.addEventListener("ready",
                        function(e) {
                          console.log("connect: " + e.type + ", ready:" + e.ready);
                          getTemperature();
					      var options = JSON.parse(window.localStorage.getItem('options'));
						  var mode = options.mode;
							if(mode=="forecast"){
								fetchTodayEffect();
							}else{
								fetchCurrentEffect();
							}
                        });

Pebble.addEventListener("appmessage",
                        function(e) {
                          console.log("message: " + JSON.stringify(e));
							if(e.payload.temperaturerequest == 1) {
								console.log("TEMPERATURE REQUEST");
								getTemperature();
							} else if (e.payload.forecastpower == 1) {
								console.log("FORECAST REQUEST");
								fetchTodayEffect();
							} else if (e.payload.currentpower == 1) {
								console.log("CURRENT REQUEST");
								fetchCurrentEffect();
							}                    
                        });
