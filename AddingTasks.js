// Thanks to Filip Dimitrovski
// https://stackoverflow.com/a/55926440/7130273
function sha512(str) {
  return crypto.subtle.digest("SHA-512", new TextEncoder("utf-8").encode(str)).then(buf => {
    return Array.prototype.map.call(new Uint8Array(buf), x=>(('00'+x.toString(16)).slice(-2))).join('');
  });
}

function filterEnter(event)
{
  if(event.keyCode == 13)
  {
    sendTask()
  }
}

function sendTask()
{
  document.querySelector("[type=button]").setAttribute("disabled", "disabled");

  // Connect to server
  ws = new WebSocket("ws://CHANGE ME TO ADDRESS:PORT")
  messageState = 0
  
  ws.onopen = () => {
    console.log("Connection opened")
    ws.send("Give Challenge plz")
  }
  
  ws.onmessage = (event) => {
    if(messageState === 0)
    {
      messageState = 1
      challenge = event.data
      salt = "WellIGuessICouldHaveUsedSomeKindOfRandomNumberGeneratorButWhatEverLoLIGuessSuchALongSaltWorksAsWellololololkthxbye"
      console.log("Data received", event.data)
      console.log("Task: ", document.getElementById('task').value)
      sha512(salt + document.getElementById('password').value).then(pwHash => {
        console.log("PW Hash: " + pwHash)
        sha512(challenge + pwHash + document.getElementById('task').value).then(challengeResponse => {
          ws.send(document.getElementById('task').value)
          ws.send(challengeResponse)
        })
      })
    }
    else
    {
      document.getElementById('answer').value = event.data
      if(event.data === "All good!")
      {
        document.getElementById('answer').style.backgroundColor = "green";
        document.getElementById('task').value = ""
      }
      else
      {
        document.getElementById('answer').style.backgroundColor = "red";
      }
      ws.close()
    }
  }
  
  ws.onclose = (event) => {
    console.log("Connection closed", event.code, event.reason, event.wasClean)
    document.querySelector("[type=button]").removeAttribute("disabled");
  }
  
  ws.onerror = () => {
    console.log("Connection closed due to error")
    document.querySelector("[type=button]").removeAttribute("disabled");
    document.getElementById('answer').value = "Server error!"
    document.getElementById('answer').style.backgroundColor = "red";
  }
}