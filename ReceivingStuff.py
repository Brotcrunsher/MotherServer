import asyncio
import websockets
import nacl.utils
import base64
import hashlib

with open("CHANGE ME TO HASHED PW LOCATION!", "r") as f:
    pw_hash = f.read()

# Define a callback function to handle incoming WebSocket messages
async def handle_websocket(websocket, path):
    try:
        while True:
            message = await websocket.recv()
            if message != "Give Challenge plz":
                await websocket.close()
                continue
            
            challenge = base64.b64encode(nacl.utils.random(size=32)).decode('ascii')
            await websocket.send(challenge)
            task = await websocket.recv()
            if len(task) == 0:
              await websocket.send("Empty Task!")
              await websocket.close()
              continue
            challenge_response_received = await websocket.recv()
            
            challenge_response_expected = hashlib.sha512((challenge + pw_hash + task).encode('utf-8')).hexdigest()
            
            # Todo: This is only because bbe::UTF8String isn't actually utf8...
            task = task.replace("ä", "ae")
            task = task.replace("ö", "oe")
            task = task.replace("ü", "ue")
            task = task.replace("Ä", "Ae")
            task = task.replace("Ö", "Oe")
            task = task.replace("Ü", "Ue")
            task = task.replace("ß", "ss")
            
            print("Task: " + task)
            print("Challenge Response Received: " + challenge_response_received)
            print("Challenge Response Expected: " + challenge_response_expected)
            
            if challenge_response_received == challenge_response_expected:
              with open("Tasks.txt", "a") as f:
                f.write(challenge_response_expected + ":" + task + "\n")
              await websocket.send("All good!")
            else:
              await websocket.send("Bad password!")
            
            await websocket.close()
    except websockets.ConnectionClosed:
        pass

if __name__ == "__main__":
    # Start the WebSocket server
    start_server = websockets.serve(handle_websocket, "localhost", 8765)

    asyncio.get_event_loop().run_until_complete(start_server)
    asyncio.get_event_loop().run_forever()
    