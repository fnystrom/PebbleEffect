
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
              "current":"--",
              "apa":"--"
          });
        console.log("Error");
      }
    }
  };
  req.send(null);
}

Pebble.addEventListener("ready",
                        function(e) {
                          console.log("connect!" + e.ready);
                          console.log(e.type);
                        });

Pebble.addEventListener("appmessage",
                        function(e) {
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


