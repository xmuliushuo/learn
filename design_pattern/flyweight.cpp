#include <iostream>
#include <vector>

using namespace std;

//棋子颜色
enum PieceColor {BLACK, WHITE};
//棋子位置
struct PiecePos
{
	int x;
	int y;
	PiecePos(int a, int b): x(a), y(b) {}
};

//棋子定义
class Piece
{
protected:
	PieceColor m_color; //颜色
public:
	Piece(PieceColor color): m_color(color) {}
	~Piece() {}
	virtual void Draw() {}
};

class BlackPiece: public Piece
{
public:
	BlackPiece(PieceColor color): Piece(color) {}
	~BlackPiece() {}
	void Draw() { cout<<"绘制一颗黑棋\n"; }
};

class WhitePiece: public Piece
{
public:
	WhitePiece(PieceColor color): Piece(color) {}
	~WhitePiece() {}
	void Draw() { cout<<"绘制一颗白棋\n";}
};

class PieceBoard
{
private:
	vector<PiecePos> m_vecPos; //存放棋子的位置
	Piece *m_blackPiece;       //黑棋棋子 
	Piece *m_whitePiece;       //白棋棋子
	string m_blackName;
	string m_whiteName;
public:
	PieceBoard(string black, string white): m_blackName(black), m_whiteName(white)
	{
		m_blackPiece = NULL;
		m_whitePiece = NULL;
	}
	~PieceBoard() { delete m_blackPiece; delete m_whitePiece;}
	void SetPiece(PieceColor color, PiecePos pos)
	{
		if(color == BLACK)
		{
			if(m_blackPiece == NULL)  //只有一颗黑棋
				m_blackPiece = new BlackPiece(color);	
			cout<<m_blackName<<"在位置("<<pos.x<<','<<pos.y<<")";
			m_blackPiece->Draw();
		}
		else
		{
			if(m_whitePiece == NULL)
				m_whitePiece = new WhitePiece(color);
			cout<<m_whiteName<<"在位置("<<pos.x<<','<<pos.y<<")";
			m_whitePiece->Draw();
		}
		m_vecPos.push_back(pos);
	}
};

int main()
{
	PieceBoard pieceBoard("A","B");
	pieceBoard.SetPiece(BLACK, PiecePos(4, 4));
	pieceBoard.SetPiece(WHITE, PiecePos(4, 16));
	pieceBoard.SetPiece(BLACK, PiecePos(16, 4));
	pieceBoard.SetPiece(WHITE, PiecePos(16, 16));
}