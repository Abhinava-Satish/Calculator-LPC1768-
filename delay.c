void delay(unsigned int ms)
{
    unsigned int i, j;
    // Adjusted loop count for more accurate timing
    // This assumes ~96MHz system clock, adjust if different
    for (i = 0; i < ms; i++)
        for (j = 0; j < 2000; j++); // Increased from 1275 to 2000
}
