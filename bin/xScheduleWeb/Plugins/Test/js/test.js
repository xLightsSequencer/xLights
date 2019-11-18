$(document).ready(function() {
loadModels();
$('#testColor').colorpicker();

if (getQueryVariable("args") == 'noheader'){
  $('#nav').css('display', 'none');
  $('#header').css('display', 'none');
  $('#footer').css('display', 'none');
}
});
testcolor = "white";
$( "#testColor" ).change(function() {
  testcolor = $( "#testColor" ).val();
});

function loadModels() {
  $.ajax({
    type: "GET",
    url: '/xScheduleStash?Command=retrieve&Key=GetModels',
    success: function(response) {
      response = JSON.parse(response);
      $('#testheadding').html("Test Mode (BETA)");

      for (var i = 0; i < response.models.length; i++) {
        var models =
          `<tr>
            <td>` + response.models[i].name +
          `</td>
          <td class="col-md-2">
          </td>
            <td class="col-md-2">

              <button type="button" onclick="runCommand('Set pixel range', '`+ response.models[i].startchannel + `,` + response.models[i].channels + `,white,')" class="btn btn-info btn-xs" name="button">On</button>
              <button type="button" onclick="setCustomColor('`+ response.models[i].startchannel + `','` + response.models[i].channels + `')" class="btn btn-info btn-xs" name="button">Color</button>
              <button type="button" onclick="runCommand('Set pixel range', '`+ response.models[i].startchannel + `,` + response.models[i].channels + `,,')" class="btn btn-default btn-xs" name="button">Reset</button>
            </td>
          </tr>`;

        $("#testModels").append(models);
      }
    }
  });
}
function setCustomColor(a,b){
  runCommand('Set pixel range', ''+ a +','+b+','+testcolor+',');
}

function getSetPixels() {
  $.ajax({
    url: '/xScheduleQuery?Query=GetPlayingStatus',
    dataType: "json",
    success: function(response) {
      console.log(response);
    }
  });
}
