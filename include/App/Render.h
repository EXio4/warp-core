
class Render {
public:
    Render();
    void updateCanvas(int width, int height);
    int* render(double timestamp);    
private:
    int width;
    int height;
    int* data;
};