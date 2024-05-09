import sys
import pygame
import asyncio
async def handle_input():
    global line_toint
    loop = asyncio.get_event_loop()
    reader = asyncio.StreamReader(loop=loop)
    reader_protocol = asyncio.StreamReaderProtocol(reader)

    await loop.connect_read_pipe(lambda: reader_protocol, sys.stdin)

    while True:
        data = await reader.readline()
        line = data.decode('utf-8').rstrip() 
        line_toint = int(float(line) * 2000)

        if line_toint > 765:
            line_toint = 765 - (line_toint/765)

async def main():
    # Initialize Pygame
    pygame.init()

    input_task = asyncio.create_task(handle_input())
    pygame_task = loop.run_in_executor(None, pygame_loop)

    await asyncio.gather(input_task, pygame_task)

    pygame.quit()
    sys.exit()

def pygame_loop():
    width, height = 500, 300
    screen = pygame.display.set_mode((width, height))
    pygame.display.set_caption("audio vis")

    clock = pygame.time.Clock()
    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
        if line_toint <= 255:
            screen.fill((0, 0, line_toint))
        if line_toint > 255 and line_toint <= 510:
            screen.fill((0, line_toint-255, 255))
        if line_toint > 510:
            screen.fill((line_toint-510,255,255))
        pygame.display.flip()
        clock.tick(240)
if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    loop.run_until_complete(main())
