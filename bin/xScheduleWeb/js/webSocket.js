// Create WebSocket connection.
url = 'ws://' + location.hostname + (location.port ? ':' + location.port : '');
const socket = new WebSocket(url);
// Connection opened
socket.addEventListener('open', function(event) {
  console.log("Socket Opened");
});

socket.addEventListener('message', function(event) {
  var response = JSON.parse(event.data);
  if (response.score == undefined && response.highscore == undefined) {
    if (response.status != undefined) {
      playingStatus = response;
    }

    if (response.result == 'failed') {
      notification('Failed: ' + response.message, 'danger', '0');
    }
  }

  if (response.reference != "" && response.reference != undefined) {
    var fun = response.reference;
    //console.log(response.reference);
    var fn = window[fun];
    if (typeof fn === "function") {
      fn(response);
    } else {
      notification('Unknown Reference Function: "' + response.reference + '"', 'danger', '0');
    }
  }
});

socket.onclose = function(e) {
  console.log('Socket Disconnected!');
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
  console.log("Test function worked!");
  console.log(response);
}
