$(document).ready(function() {
    loadPlaylists();
    loadFSEQs();

    if (getQueryVariable("args") == 'noheader') {
        $('#nav').css('display', 'none');
        $('#header').css('display', 'none');
        $('#footer').css('display', 'none');
    }
});


function loadPlaylists() {
    $.ajax({
        type: "GET",
        url: '/xScheduleQuery?Query=GetPlayLists',
        success: function(response) {
            removeOptions(document.getElementById("playListSelect"));
            response = JSON.parse(JSON.stringify(response));
            $('#testheadding').html("PlayList Editor");
            var drpplayList = document.getElementById("playListSelect");

            for (var i = 0; i < response.playlists.length; i++) {
                var option = document.createElement("OPTION");
                option.innerHTML = response.playlists[i].name;
                option.value = response.playlists[i].id;
                drpplayList.options.add(option);
            }
            loadPlaylistSteps();
        }
    });
}

function loadPlaylistSteps() {
    var plylst = $("#playListSelect :selected").val();
    $.ajax({
        type: "GET",
        url: '/xScheduleQuery?Query=GetPlayListSteps&Parameters=id:' + plylst,
        success: function(response) {
            response = JSON.parse(JSON.stringify(response));
            var table = document.getElementById("playlistSteps");
            while (table.rows.length > 1) {
                table.deleteRow(1);
            }
            for (var i = 0; i < response.steps.length; i++) {
                var steps =
                    `<tr>
            <td>` + response.steps[i].name +
                    `</td>
          <td class="col-md-2">` + response.steps[i].length +
                    `</td>
            <td class="col-md-2">
              <button type="button" onclick="moveUp('id:` + response.steps[i].id + `')" class="btn btn-info btn-xs" name="button">Move Up</button>
              <button type="button" onclick="moveDown('id:` + response.steps[i].id + `')" class="btn btn-info btn-xs" name="button">Move Down</button>
              <button type="button" onclick="deleteStep('id:` + response.steps[i].id + `','` + response.steps[i].name + `')" class="btn btn-default btn-xs" name="button">Delete Step</button>
            </td>
          </tr>`;
                $("#playlistSteps").append(steps);
            }
        }
    });
}

function loadFSEQs() {
    $.ajax({
        type: "GET",
        url: '/xScheduleQuery?Query=listfilesinshowfolder&Parameters=*.fseq',
        success: function(response) {

            removeOptions(document.getElementById("playListAdd"));
            response = JSON.parse(JSON.stringify(response));
            var drpplayList = document.getElementById("playListAdd");
            for (var i = 0; i < response.files.length; i++) {
                var option = document.createElement("OPTION");
                option.innerHTML = response.files[i];
                option.value = response.files[i];
                drpplayList.options.add(option);
            }
        }
    });
}

function addPlaylist() {
    var playlst = prompt("Please Enter Playlist Name", "New Playlist");
    if (playlst != null && playlst != "") {
        runCommand('Add playlist', '' + playlst + '');
        loadPlaylists();
    }
}

function addFSEQ() {
    var fseq = $("#playListAdd :selected").val();
    var play = $("#playListSelect :selected").val();
    runCommand('Add playlist step fseq', 'id:' + play + ',' + fseq + '');
    loadPlaylistSteps();
}

function removePlaylist() {
    var play = $("#playListSelect :selected").val();
    var playname = $("#playListSelect :selected").text();
    if (confirm('Are you sure you want to Remove Playlist "' + playname + '" ?')) {
        runCommand('Delete playlist', 'id:' + play + '');
        loadPlaylists();
    }
}

function moveUp(stp) {
    var play = $("#playListSelect :selected").val();
    runCommand('Move up playlist step', 'id:' + play + ',' + stp + '');
    loadPlaylistSteps();
}

function moveDown(stp) {
    var play = $("#playListSelect :selected").val();
    runCommand('Move down playlist step', 'id:' + play + ',' + stp + '');
    loadPlaylistSteps();
}

function deleteStep(stpid, stdname) {
    var play = $("#playListSelect :selected").val();
    if (confirm('Are you sure you want to Remove Playlist Step "' + stdname + '" ?')) {
        runCommand('Delete playlist step', 'id:' + play + ',' + stpid + '');
        loadPlaylistSteps();
    }
}

function removeOptions(selectElement) {
    var i, L = selectElement.options.length - 1;
    for (i = L; i >= 0; i--) {
        selectElement.remove(i);
    }
}