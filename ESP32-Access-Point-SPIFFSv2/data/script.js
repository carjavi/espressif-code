// Get current sensor readings when the page loads  
window.addEventListener('load', getReadings);

// Function to get current readings on the webpage when it loads for the first time
function getReadings(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);
      var temp = myObj.temperature;
      var hum = myObj.humidity;
      var time = myObj.time;
      document.getElementById("temp").innerHTML = temp;
      document.getElementById("humi").innerHTML = hum;
      document.getElementById("time").innerHTML = time;
    }
  }; 
  xhr.open("GET", "/readings", true);
  xhr.send();
}


if (!!window.EventSource) {
  var source = new EventSource('/events');
  
  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
  
  source.addEventListener('message', function(e) {
    console.log("message", e.data);
  }, false);
  
  source.addEventListener('new_readings', function(e) {
    console.log("new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    document.getElementById("temp").innerHTML = myObj.temperature;
    document.getElementById("humi").innerHTML = myObj.humidity;
    document.getElementById("time").innerHTML = myObj.time;
  }, false);
}