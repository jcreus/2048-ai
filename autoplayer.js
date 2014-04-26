var __triggerKeyboardEvent = function(el, keyCode) {
  var eventObj = document.createEventObject ? document.createEventObject() : document.createEvent("Events");
  if(eventObj.initEvent) eventObj.initEvent("keydown", true, true);
  eventObj.keyCode = keyCode;
  eventObj.which = keyCode;
  el.dispatchEvent ? el.dispatchEvent(eventObj) : el.fireEvent("onkeydown", eventObj); 
};
var move = function(direction) {
  if (direction === "left") __triggerKeyboardEvent(document.body, 37);
  else if (direction === "up") __triggerKeyboardEvent(document.body, 38);
  else if (direction === "right") __triggerKeyboardEvent(document.body, 39);
  else if (direction === "down") __triggerKeyboardEvent(document.body, 40);
};
var bestMove = function(gameState) {
  var items = ["left", "up", "right", "down"];
  return items[Math.floor(Math.random()*items.length)];
};
var gameState = function() {
  var state = JSON.parse(window.localStorage.gameState);
  var v = [];
  for (var y = 0; y < 4; ++y)
    for (var x = 0; x < 4; ++x) {
      if (state.grid.cells[x][y] === null) v.push(0);
      else v.push(state.grid.cells[x][y].value);
    }
  return v.join(" ");
}
var doHackyAjax = function(gameState) {
  var script = document.createElement('script');
  var state = encodeURIComponent(gameState);
  script.src = "http://<<<HOST>>>:<<<PORT>>>/move/?state=" + state + "&callback=hackyAjaxCallback";
  document.body.appendChild(script);
}
var hackyAjaxCallback = function(data) {
  move(data);
  setTimeout(mainLoop, 50);
}
var mainLoop = function() {
  doHackyAjax(gameState());
}
mainLoop();
