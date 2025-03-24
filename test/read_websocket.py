import asyncio
import websockets

async def read_websocket():
    async with websockets.connect("ws://localhost:9002") as websocket:
        i = 0
        while True:
            message = await websocket.recv()
            if i % 100 == 0:
                print(message)

asyncio.run(read_websocket())