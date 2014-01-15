
function fetchEffect() {
  var response;
  var req = new XMLHttpRequest();
  req.open('GET', "http://effektprognos.azurewebsites.net/Home/PebbleEffect", true);
  req.onload = function(e) {
    if (req.readyState == 4) {
      if(req.status == 200) {
        console.log(req.responseText);
        response = JSON.parse(req.responseText);

        console.log(response.current);

        var current, estimate;
        if (response) {


          current = response.current + "kWh";
          estimate = response.estimate + "kWh";

          console.log("X Effekt", current + ", " + estimate);
			
          Pebble.sendAppMessage({
              "current":current,
              "apa":estimate});
        }
      } else {
          Pebble.sendAppMessage({
              "current":0,
              "apa":0
          });
        console.log("Error");
      }
    }
  };
  req.send(null);
}

function fetchApa() {
/*    Pebble.sendAppMessage({
        "estimate":"42"
    },
  function(e) {
    console.log("Successfully delivered message with transactionId=" + e.data.transactionId);
  },
  function(e) {
    console.log("Unable to deliver message with transactionId=" + e.data.transactionId);
  });
	*/
	Pebble.sendAppMessage({
        "apa":"4","current":"13"
    },
  function(e) {
    console.log("Successfully delivered message with transactionId=" + e.data.transactionId);
  },
  function(e) {
    console.log("Unable to deliver message with transactionId=" + e.data.transactionId);
  });
	/*
	Pebble.sendAppMessage({
        "current":"13"
    },
  function(e) {
    console.log("Successfully delivered message with transactionId=" + e.data.transactionId);
  },
  function(e) {
    console.log("Unable to deliver message with transactionId=" + e.data.transactionId);
  });
  */
}

Pebble.addEventListener("ready",
                        function(e) {
                          console.log("connect!" + e.ready);
							//fetchApa();
							//fetchEffect();
                          console.log(e.type);
                        });

Pebble.addEventListener("appmessage",
                        function(e) {
                          //fetchApa();
                          fetchEffect();
                          console.log(e.type);
                          console.log("message!");
                        });

Pebble.addEventListener("webviewclosed",
                                     function(e) {
                                     console.log("webview closed");
                                     console.log(e.type);
                                     console.log(e.response);
                                     });


