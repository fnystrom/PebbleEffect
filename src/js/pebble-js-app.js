
function fetchEffect() {
  var response;
  var req = new XMLHttpRequest();
  req.open('GET', "http://effektprognos.azurewebsites.net/Home/PebbleEffect", true);
  req.onload = function(e) {
    if (req.readyState == 4) {
      if(req.status == 200) {
        console.log(req.responseText);
        alert(req.responseText);
        response = JSON.parse(req.responseText);
        var current, estimate;
        if (response && response.list && response.list.length > 0) {
          var effectResult = response.list[0];
          current = effectResult.main.currrent;
          estimate = effectResult.main.estimate;
          
          Pebble.sendAppMessage({
              "current": current,
              "estimate": estimate
          });
        }
      } else {
          Pebble.sendAppMessage({
              "current": 0,
              "estimate": 0
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
                          fetchEffect();
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


