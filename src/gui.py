import pygame
import os
import sys
import gomoku

EMPTY = 0
BLACK = 1
WHITE = 2
BLACK_COLOR = [0, 0, 0]
WHITE_COLOR = [255, 255, 255]
BOARD_SIZE = 15
BORDER_WIDTH = 1
GRID_SIZE = 40

background_img = pygame.image.load(os.path.join('../assets', 'background.jpg'))

class GomokuBoard:
    def __init__(self):
        self._board = [[EMPTY] * BOARD_SIZE for _ in range(BOARD_SIZE)]

    def reset(self):
        for row in range(BOARD_SIZE):
            self._board[row] = [EMPTY] * BOARD_SIZE

    def move(self, row, col, is_black):
        if self._board[row][col] == EMPTY:
            self._board[row][col] = BLACK if is_black else WHITE
            return True
        return False

    def draw(self, screen):
        for h in range(1, BOARD_SIZE + 1):
            pygame.draw.line(screen, BLACK_COLOR, [GRID_SIZE, h * GRID_SIZE], [BOARD_SIZE * GRID_SIZE, h * GRID_SIZE], 1)
            pygame.draw.line(screen, BLACK_COLOR, [h * GRID_SIZE, GRID_SIZE], [h * GRID_SIZE, BOARD_SIZE * GRID_SIZE], 1)

        pygame.draw.rect(screen, BLACK_COLOR, [GRID_SIZE - BORDER_WIDTH, GRID_SIZE - BORDER_WIDTH, (BOARD_SIZE + 1) * GRID_SIZE, (BOARD_SIZE + 1) * GRID_SIZE], BORDER_WIDTH)

        pygame.draw.circle(screen, BLACK_COLOR, [GRID_SIZE * 8, GRID_SIZE * 8], 4, 0)  # Central
        for x in [GRID_SIZE * 4, GRID_SIZE * 12]:
            for y in [GRID_SIZE * 4, GRID_SIZE * 12]:
                pygame.draw.circle(screen, BLACK_COLOR, [x, y], 3, 0)

        for row in range(BOARD_SIZE):
            for col in range(BOARD_SIZE):
                if self._board[row][col] != EMPTY:
                    color = BLACK_COLOR if self._board[row][col] == BLACK else WHITE_COLOR
                    pos = [GRID_SIZE * (col + 1), GRID_SIZE * (row + 1)]
                    pygame.draw.circle(screen, color, pos, 18, 0)


def main():
    p1 = gomoku.Player1()
    p2 = gomoku.Player2()
    game = gomoku.Gomoku(p1, p2)

    pygame.init()
    size = width, height = (BOARD_SIZE + 1) * GRID_SIZE, (BOARD_SIZE + 1) * GRID_SIZE
    screen = pygame.display.set_mode(size)
    pygame.display.set_caption("Gomoku")

    board = GomokuBoard()
    clock = pygame.time.Clock()
    is_black_turn = True

    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

            elif event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
                x, y = event.pos
                row = round(y/GRID_SIZE) - 1
                col = round(x/GRID_SIZE) - 1

                if is_black_turn:
                    p1.x = row
                    p1.y = col
                    p1.makeMoves(game)

                else:
                    p2.x = row
                    p2.y = col
                    p2.makeMoves(game)

                if row >= 0 and col >= 0 and row < BOARD_SIZE and col < BOARD_SIZE:
                    if board.move(row, col, is_black_turn):
                        is_black_turn = not is_black_turn

        screen.fill([255, 255, 255])
        screen.blit(background_img, (0, 0))
        board.draw(screen)
        pygame.display.flip()

        if game.state == 1:  # We have a winner.
            running = False
            winner = "BLACK" if game.current_player == 1 else "WHITE"
            print(f"{winner} wins! Exiting the game.")
            pygame.time.wait(2000)

        clock.tick(60)

    pygame.quit()
    sys.exit()

if __name__ == '__main__':
    main()



