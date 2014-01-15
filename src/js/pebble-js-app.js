
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
        "current":13,
        "estimate":42
    });
}

Pebble.addEventListener("ready",
                        function(e) {
                          console.log("connect!" + e.ready);
                          fetchApa();
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


