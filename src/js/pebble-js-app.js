function appMessageAck(e) {
    //console.log("options sent to Pebble successfully");
}

function appMessageNack(e) {
    //console.log("options not sent to Pebble: " + e.error.message);
}

Pebble.addEventListener("showConfiguration", function() {
    var options = JSON.parse(window.localStorage.getItem('options'));
    //console.log("read options: " + JSON.stringify(options));
    //console.log("showing configuration");
	var uri;
	
    if (options === null) {
        uri = 'http://effektprognos.azurewebsites.net/eliqeffekt.html';
    } else {
        uri = 'http://effektprognos.azurewebsites.net/eliqeffekt.html?' + 'accesstoken=' + encodeURIComponent(options.accesstoken)+ '&background=' + encodeURIComponent(options.background);
    }
    Pebble.openURL(uri);
});

Pebble.addEventListener("webviewclosed", function(e) {
    //console.log("configuration closed");
    if (e.response !== '') {
        var options = JSON.parse(decodeURIComponent(e.response));
		//console.log("storing options: " + JSON.stringify(options));
        window.localStorage.setItem('options', JSON.stringify(options));
		var background = {'background':options.background};
        //Pebble.showSimpleNotificationOnPebble("conf", JSON.stringify(background));
		//console.log("sending options: " + JSON.stringify(background));
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

function fetchEliqEffect() {
  var response;
  var req = new XMLHttpRequest();
  var accesstoken="";
  var options = JSON.parse(window.localStorage.getItem('options'));
  if (options === null) {
    Pebble.sendAppMessage({"current":"--","apa":"--"});
    return;
  } else {
    if (options.accesstoken === null){
      Pebble.sendAppMessage({"current":"--","apa":"--"});
      return;
    } else {
      accesstoken = options.accesstoken;	
    }
  }

  req.open('GET', "https://my.eliq.se/api/data?accesstoken="+accesstoken+"&startdate="+today()+"&intervaltype=hour", true);
  console.log("Calling eliq...");
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
                        
		console.log("current=" + current + ", estimate=" + estimate);
			
        Pebble.sendAppMessage({
              "current":current,
              "apa":estimate});
        }
      } else {
        Pebble.sendAppMessage({
              "current":"--",
              "apa":"--"});
        console.log("Error");
      }
    }
  };
  req.send(null);
}

Pebble.addEventListener("ready",
                        function(e) {
                          console.log("connect: " + e.type + ", ready:" + e.ready);
                          fetchEliqEffect();
                        });

Pebble.addEventListener("appmessage",
                        function(e) {
                          console.log("message: " + e.type);
                          fetchEliqEffect();
                        });
