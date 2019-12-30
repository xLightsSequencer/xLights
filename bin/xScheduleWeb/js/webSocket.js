// Create WebSocket connection.
url = 'ws://' + location.hostname + (location.port ? ':' + location.port : '');
const socket = new ReconnectingWebSocket(url);
// Connection opened
socket.addEventListener('open', function(event) {
  console.log("Socket Opened");
});

socket.addEventListener('message', function(event) {
  var response = JSON.parse(event.data);
  if (response.score == undefined && response.highscore == undefined) {
    if (response.status != undefined) {
      playingStatus = response;
      updateNavStatus();
	  callRegisteredForStatus(response);
    }

    if (response.result == 'failed') {
      notification('Failed: ' + response.message, 'danger', '0');
    }
  }

  //Run Fuction if refrence is set
  if (response.reference != "" && response.reference != undefined) {
    var fun = response.reference;
    var fn = window[fun];
    if (typeof fn === "function") {
      fn(response);
    } else {
      notification('Unknown Reference Function: "' + response.reference + '"', 'danger', '0');
    }
  }
  if (response.result == 'not logged in') {
    window.location.href = "login.html";
  }
  if (response.result == 'failed' && response.message == 'Login failed.') {
    window.location.href = "login.html";
  }
});

socket.onclose = function(e) {
  notification('Web Socket: Disconnected "' + e + '"', 'danger', '2');
};

//
// Listen for messages
// socket.addEventListener('message', function(event) {
//   //console.log('Message from server', JSON.parse(event.data));
//   var response = JSON.parse(event.data);
//   if (response.reference != undefined) {
//     console.log(response.reference);
//     var fun = "test";
//     var fn = window[fun];
//     fn("");
//   }
// });

function test(response) {
  notification('Test Function: "' + response + '"', 'success', '0');
}
