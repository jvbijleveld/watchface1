var APPID = 'd8be18b5726a69e2fc838d789b558850';

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' + pos.coords.latitude + '&lon=' + pos.coords.longitude + '&appid=' + APPID;
  xhrRequest(url, 'GET', function(responseText) {
    console.log(responseText);
      
    var weather = JSON.parse(responseText);
    var temperature = Math.round(weather.main.temp - 273.15);
    var conditions = weather.weather[0].main;
      
    console.log(temperature + " " + conditions);
    var dictionary = {
        "KEY_TEMPERATURE": temperature,
        "KEY_CONDITIONS": conditions
    };
      
    Pebble.sendAppMessage(dictionary,
      function(e) {
        console.log('Weather info sent to Pebble successfully!');
      },
      function(e) {
        console.log('Error sending weather info to Pebble!');
      }
    );
  });
}

function locationError(err) {
  console.log("Error requesting location!");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");
    // Get the initial weather
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getWeather();
  }                     
);
