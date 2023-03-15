int putchar(int);

int c = 4;

int main()
{
    putchar(c);
    {
        int c = 9;
        putchar(c);
        {
            int c = 20;
            putchar(c);
        }
    }
}
