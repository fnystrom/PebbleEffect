
function fetchEffect() {
  var response;
  var req = new XMLHttpRequest();
  req.open('GET', "http://effektprognos.azurewebsites.net/Home/PebbleEffect", true);
  req.onload = function(e) {
    if (req.readyState == 4) {
      if(req.status == 200) {
        console.log(req.responseText);
        response = JSON.parse(req.responseText);
        var current, estimate;
        if (response && response.list && response.list.length > 0) {
          var effectResult = response.list[0];

          current = effectResult.currrent;
          estimate = effectResult.estimate;
          
          Pebble.showSimpleNotificationOnPebble("Effekt", current + ", " + estimate);
			
          Pebble.sendAppMessage({
              "current":current,
              "estimate":estimate});
        }
      } else {
          Pebble.sendAppMessage({
              "current":0,
              "estimate":0
          });
        console.log("Error");
      }
    }
  };
  req.send(null);
}

function fetchApa() {
    Pebble.sendAppMessage({
        "estimate":"42"
    },
  function(e) {
    console.log("Successfully delivered message with transactionId=" + e.data.transactionId);
  },
  function(e) {
    console.log("Unable to deliver message with transactionId=" + e.data.transactionId /*+ " Error is: " + e.error.message*/);
  });
	
	Pebble.sendAppMessage({
        "apa":"4"
    },
  function(e) {
    console.log("Successfully delivered message with transactionId=" + e.data.transactionId);
  },
  function(e) {
    console.log("Unable to deliver message with transactionId=" + e.data.transactionId + " Error is: " + e.error);
  });
	
	Pebble.sendAppMessage({
        "current":"13"
    },
  function(e) {
    console.log("Successfully delivered message with transactionId=" + e.data.transactionId);
  },
  function(e) {
    console.log("Unable to deliver message with transactionId=" + e.data.transactionId + " Error is: " + e);
  });
}

Pebble.addEventListener("ready",
                        function(e) {
                          console.log("connect!" + e.ready);
							//fetchEffect();
                          console.log(e.type);
                        });

Pebble.addEventListener("appmessage",
                        function(e) {
                          fetchApa();
                          console.log(e.type);
                          console.log("message!");
                        });

Pebble.addEventListener("webviewclosed",
                                     function(e) {
                                     console.log("webview closed");
                                     console.log(e.type);
                                     console.log(e.response);
                                     });


