// define XMLHttpRequest to get info via API
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
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
      pos.coords.latitude + "&lon=" + pos.coords.longitude;

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      // Temperature in Kelvin requires adjustment
      var temperature = Math.round(json.main.temp - 273.15);
      console.log("Temperature is " + temperature);

      // Conditions
      var conditions = json.weather[0].main;      
      console.log("Conditions are " + conditions);
      
      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_TEMPERATURE": temperature,
        "KEY_CONDITIONS": conditions
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }      
  );
}

function locationError(err) {
  console.log("Error requesting location!");
}

// send email notifications when stock price increases more than 5%
function emailNotificationUp(){
    // data to be sent to mandrill api via XMLHttpRequest Post
    // here post data require a registered API key
    // essential message to be sent in email including sender, receiver, subject, contents.
    var data = {
    "key": "ed_kRGLKRL2al2Oe17B5CA",
    "message": {
      "from_email": "lizhi.magic@gmail.com",
      "from_name": "Zhi Li",
      "to": [{"email": "lizhi.magic@gmail.com", "name":"Painkiller","type":"to"}],
      "subject": "[^w^]Target Stock is Soaring!",
      "text": "Notice Change in your PEBBLE WATCH :)\nYour target stock is soaring!"
    }
  };

   // stringify data to Post format
   var postdata = JSON.stringify(data);
   console.log(postdata);

   // initialize Http Request, define API url
   var x = new XMLHttpRequest();
   var url = "https://mandrillapp.com/api/1.0/messages/send.json";
  
   x.open('POST', url, true);
   x.onreadystatechange = function() {
        if (x.readyState == 4) {
        console.log(x.status);
        console.log(x.responseText);
        }
    };

   // send request
  x.send(postdata);
  
}

// send email notification when price decrease more than 5%
function emailNotificationDown(){
  // store key & message info required by Mandrill API in data
    var data = {
    "key": "ed_kRGLKRL2al2Oe17B5CA",
    "message": {
      "from_email": "lizhi.magic@gmail.com",
      "from_name": "Zhi Li",
      "to": [{"email": "lizhi.magic@gmail.com", "name":"Painkiller","type":"to"}],
      "subject": "[TAT]Target Stock is Dropping!",
      "text": "Notice Change in your PEBBLE WATCH :(\nYour target stock is dropping quickly."
    }
  };
  
  // reformat data to send by XMLHttpRequest.send
   var postdata = JSON.stringify(data);
   console.log(postdata);
   var x = new XMLHttpRequest();
  // mandrill api json file
   var url = "https://mandrillapp.com/api/1.0/messages/send.json";
  
   x.open('POST', url, true);
  // callback responsText, displayed on app log, check whether communication is established
   x.onreadystatechange = function() {
        if (x.readyState == 4) {
        console.log(x.status);
        console.log(x.responseText);
        }
    };
  // send data to enable notification email when condition fits
  x.send(postdata);
  
}


// Get Stock information

// construct query url
function stockSuccess() {
  // Construct URL, fetch data from "PBR","YHOO","NOK","GOOG","MSFT"
  var url = "https://query.yahooapis.com/v1/public/yql?q=select%20*%20from%20yahoo.finance.quotes%20where%20symbol%20in%20(%22PBR%22%2C%22YHOO%22%2C%22NOK%22%2C%22GOOG%22%2C%22MSFT%22)%0A%09%09&format=json&diagnostics=true&env=http%3A%2F%2Fdatatables.org%2Falltables.env&callback=";

  // Send request to Yahoo Finance
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with stock quotes info
      var json = JSON.parse(responseText);
      // 0 for PBR, 1 for YHOO, 2 for AAPL, etc.
      var stock = json.query.results.quote[0];
        
      // record symbol information
      var symbol = stock.symbol;
      console.log("Symbol: " + symbol);
        
      // record Ask value
      var askValue = stock.AskRealtime;
      console.log("Ask value: "+ askValue);
        
      // record percent change
      var changeRealtime = stock.ChangeRealtime;
      console.log("Change value: "+ changeRealtime);
      
      // define flag for face display
      var flag = 0;
      if (changeRealtime < 0){
          flag = 1; //sad face
        }
      else {
          flag = 0; //happy face       
      }
      
      var rate = 2;
      if (askValue == changeRealtime){
        flag = 0;
      } else {
        rate = askValue / (askValue - changeRealtime);
      }
      
      console.log("Change Rate: "+ rate);
      
      // increase by 5%
      if (rate >= 1.05){
        flag = 2; //$$ more happy face
        emailNotificationUp();
      }
      // decrease by 5%
      else if (rate <= 0.95 && rate > 0){
        flag = 3; //even more sad face
        emailNotificationDown();
      }
            
      // Assemble dictionary using our keys
      // KEY_FACE is used to transfer flag value to pebble, used by face_change()
      var dictionary = {
        "KEY_SYMBOL": symbol,
        "KEY_CURRENT": askValue,
        "KEY_CHANGE": changeRealtime,
        "KEY_FACE": flag
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("stock info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending stock info to Pebble!");
        }
      );
    }      
  );
}

function getWeather() {
  window.navigator.geolocation.getCurrentPosition(
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
    
    // Get the initial stock info
    stockSuccess();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getWeather();
    stockSuccess();
  }                     
);
