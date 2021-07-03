var jsonObj;
var editor;

$(document).ready(function () {

  // Sobald das Dokument geladen ist
  console.log("I'm ready to makesmart!");


  var page = window.location.pathname.split("/").pop();
  if (page == "edit" || page == "edit.html") {
    jsonObj = $('#json-input').html();
    jsonObj = JSON.parse($('#json-input').html());
    initJsonEditor(jsonObj);

    console.log("scrolL!");
    $("html, body").animate({
      scrollTop: $(
        'html, body').get(0).scrollHeight
    }, 3000);

  }

});


/* Websockets Beispiel */

let socket = new WebSocket(`ws://${window.location.hostname}/ws`);

//  Funktionen die genutzt werden können
//  Nachricht an den Server senden: socket.send("connection:success"); 

function activateButtons() {

  $('.btn').click(function () {
    var shortcut = this.id;
    var btnGroup = $(this).attr('btn-group');

    socket.send(shortcut);

    // Button Group
    if (btnGroup) {
      $('button[btn-group="' + btnGroup + '"]').removeClass("active");
    }


    if ($(this).hasClass("switch")) {
      console.log("Its a switch!");
      if ($(this).hasClass("active")) {
        console.log("IS active");
        $(this).removeClass("active");
      } else {
        console.log("isnt active");
        $(this).addClass("active");
      }

    }

    if ($(this).hasClass("button")) {
      var color = $(this).css("background-color");
      $(this).fadeOut(200).fadeIn(200);

    }

  });

}








socket.onmessage = function (event) {
  console.log(event.data);

  try {
    var buttons = JSON.parse(event.data);
  }
  catch (e) {
    alert("Deine Konfiguration ist fehlerhaft. Behebe die Fehler um dein ESPDeck zu nutzen.");
    return;
  };

  debugOnMessage(buttons);


};



/* END EXAMPLE */

socket.onopen = function (e) {
  console.log("Websockets Verbindung hergstellt!");
  socket.send("connection:new");
  console.log();
};



/* PRETTIER */

$('#prettify').click(function () {
  try {
    var json = editor.get();
    console.log(json);
    saveData(json);
  } catch (e) {
    alert("Du hast einen Fehler in deinem JSON!");
    return;
  }



});

function saveData(json) {
  var data = JSON.stringify(json);
  console.log(data);
  $.post("/api/post", { data }, function (result) {
    if (result == "true") {
      alert("Daten wurden gespeichert!");
    } else {
      alert("Fehler beim Speichern der Daten!");
    }
  });

}


/**

  {
    "buttons": [
      {
        "hotkey": "strg+shift+b",
        "icon": "desktop",
        "name": "Screen 2",
        "type": "switch",
        "group": 1
      }
    ]
  } 


  {"buttons": [{"hotkey": "strg+shift+b","icon": "desktop","name": "Screen 2","type": "switch","group": 1}]}

 */

function debugOnMessage(json) {
  var page = window.location.pathname.split("/").pop();
  if (page == "edit" || page == "edit.html") return;


  $('div#deck-buttons-container').html("");
  try {
    json.buttons.forEach(element => {
      console.log(element);

      // Create parent DIV of Button
      var parentDiv = $("<div>", {
        "class": "col-xs",
        "style": "margin-left: 10px;"
      });

      // Create Button
      var deckButton = $("<button>", {
        "class": "col-xs btn btn-primary trigger " + element.type,
        "id": element.hotkey
      });

      // Is ICON set in JSON?
      if (element.icon) {
        var buttonIcon = $("<i>", {
          "class": "fa fa-" + element.icon,
        });
        $(deckButton).append(buttonIcon);
      }

      // Append Button name to button
      $(deckButton).append(" " + element.name);

      // Is button in a group
      if (element.group) {
        $(deckButton).attr('btn-group', "group-" + element.group);
      }


      // Move it all togehter
      $(parentDiv).append(deckButton);
      $('div#deck-buttons-container').append(parentDiv);




    });
  } catch (e) {
    alert("Es wurden keine Buttons gefunden. Bitte konfiguriere Buttons in den Einstellungen.");
    return;
  }


  activateButtons();

}


function initJsonEditor(json) {
  var container = document.getElementById("jsoneditor");
  var options = {
    mode: 'code'
  };
  editor = new JSONEditor(container, options, json);
}