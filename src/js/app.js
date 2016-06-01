var APPID = 'dj0yJmk9ZW1iVzIxcENjamNqJmQ9WVdrOU9HUnRRVlJ1Tm5FbWNHbzlNQS0tJnM9Y29uc3VtZXJzZWNyZXQmeD1lYQ--';
var yahooUrl = 'https://query.yahooapis.com/v1/public/yql?q=<wsql>&format=json&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys';


var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};


function locationSuccess(pos) {
  // Construct URL
  //var query = 'select * from geo.places where text = "' + pos.coords.latitude + ',' + pos.coords.longitude + '"';
  var query = 'select * from geo.places where text = "almere"';
 
  // Send request to OpenWeatherMap
  xhrRequest(yahooUrl.replace('<wsql>',query), 'GET', function(responseText) {
      // responseText contains a JSON object with weather info
    console.log(yahooUrl.replace('<wsql>',query));
    console.log(responseText);
    
    var json = JSON.parse(responseText);
    console.log(json.query.results.place[0].woeid);
    
    var query = 'select * from weather.forecast where woeid ="' + json.query.results.place[0].woeid + '"';
    xhrRequest(yahooUrl.replace('<wsql>',query), 'GET', function(responseText) {
      console.log(responseText);
      
      var weather = JSON.parse(responseText);
      var temperature = Math.round((weather.query.results.channel.item.condition.temp - 32) * 5 / 9);
      var conditions = weather.query.results.channel.item.condition.text;
      
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
    
  });

}


function getFixxedWeather(){
  var wsql = 'select * from weather.forecast where woeid=WID and u="'+DEG+'"';
  var url = "https://query.yahooapis.com/v1/public/yql?q=" + wsql + "&format=json&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys";
  
  console.log(url);
      
  xhrRequest(url, 'GET', function(resp){
    console.log(resp);
    var weather = JSON.parse(resp);
  	if(weather.query.count == 1){
  	  var item = weather.query.results.channel.item.condition;
         
      var dictionary = {
           "KEY_TEMPERATURE": item.temp,
           "KEY_CONDITIONS": item.text
      };
  	} else {
  	  console.log("Error retrieving weather data!");
  	}
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
