var ipAddress = "";
window.onload = function() {
  window.setInterval(function() {
    checkLogInStatus();
  }, 1000);
}

function checkLogInStatus() {
  $.ajax({
    url: '/xScheduleQuery?Query=GetPlayingStatus',
    dataType: "json",
    success: function(response) {
      if (response.result != "not logged in") {
        window.location.href = "index.html";
        console.log("logged in");
      } else if (response.result == "not logged in") {
        ipAddress = response.ip;
      }
    }
  });
}

function loginSubmit() {
  var password = $("#webPassword")[0].value;
  var hash = MD5(ipAddress + password);
  $.ajax({
    url: '/xScheduleLogin?Credential=' + hash,
    dataType: "json",
    success: function(response) {
      console.log(response);
      if (response.result == "failed") {
        var message = "<h5>" + response.message + "</h5>";
        $("#error").html(message);
        $("#webPassword")[0].value = "";
      } else if (response.result == "ok") {
        var message = "<h5>Logging In</h5>";
        $("#error").html(message);
        window.location.href = 'login.html';
      }
    },
    error: function(error) {
      $("#error").html("ERROR: " + error);
    }
  });
}

function forgotPassword() {
  window.alert("Reset Password: Change password in | Tools - Options | in GUI");
}
