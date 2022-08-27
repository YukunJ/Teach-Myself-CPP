extern int secret;

class Adder {
public:
    explicit Adder(int x) noexcept;
    
    int add(int y) noexcept;
private:
    int secret_number;
};

