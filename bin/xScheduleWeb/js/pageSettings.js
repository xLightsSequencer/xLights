//Setting Page


function settingsLoadSettings() {
  //Populate Settings Page
  retrieveKey('webName', function(resp) {
    value = "xLights Scheduler";
    if (resp == "\n" || resp == null || resp == "") {
      //console.log("Default Setting Loaded "+ value);
    } else {
      value = resp;
    }
    $("#webName")[0].value = value;
    document.title = value;
  });
  retrieveKey('webColor', function(resp) {
    value = "#ff0000";
    if (resp != "") {
      value = resp;
    }
    $("#webColor")[0].value = value;
    $("#webColorDisplay").css("background-color", value)
  });
  retrieveKey('webLoginRequire', function(resp) {
    r = resp.split('\n');
    if (r[0] == "true") {
      $("#webLoginRequire").prop("checked", true).uniform('refresh');
    } else {
      $("#webLoginRequire").prop("checked", false).uniform('refresh');
    }
  });

  retrieveKey('webUsername', function(resp) {
    value = "";
    if (resp != "") {
      value = resp;
    }
    $("#webUsername")[0].value = value;
  });
  retrieveKey('webPassword', function(resp) {
    value = "";
    if (resp != "") {
      value = resp;
    }
    $("#webPassword")[0].value = value;
  });
}

function settingsUpdateWebSettings() {

  var inputs = $('#webSettings :input')
  var dataArr = [];
  for (var i = 0; i < inputs.length; i++) {
    dataArr.push({
      'name': inputs[i].id,
      'value': inputs[i].value
    });
  }
  for (var i = 0; i < dataArr.length; i++) {
    storeKey(dataArr[i].name, dataArr[i].value);
  }
  //prevent Page Reload
  $("#webSettings").submit(function(e) {
    e.preventDefault();
  });
  //Reload Current Page Information
  loadPageSettings();
  settingsLoadSettings();


  // for (var data in dataArr) {
  //   console.log("saving data: " + data);
  //     storeKey(data.name, data.value);
  // };

}
