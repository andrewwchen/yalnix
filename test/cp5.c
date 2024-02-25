#include <yuser.h>
int TERMINAL_MAX_LINE = 1024;
int delay = 1;
int rc = -2;
int status = -2;
int pid = -2;
int main(void)

{
    // Create parent and child, reader and writer
    TracePrintf(0,"PARENT (READER): Fork()\n");
    rc = Fork();
    if (rc == 0) {
        TracePrintf(0,"CHILD (WRITER): Fork() rc=%d\n", rc);

        // TEST 1: parent reads before line is available
        int child_tty_id = 0;
        int child_len = 23;
        char *child_buf = malloc(40*sizeof(char));
        child_buf = "Type the phrase '123':\nWRONGWRONGWRONG";
        TracePrintf(0, "CHILD (WRITER): TtyWrite(%d, %s, %d)\n", child_tty_id, child_buf, child_len);
        TtyWrite(child_tty_id, child_buf, child_len);
        TracePrintf(0, "CHILD (WRITER): expect 123 in terminal 0");

        
        child_tty_id = 1;
        child_len = 1295;
        child_buf = malloc(1295*sizeof(char));
        child_buf = "Number 15, Burger King Foot Lettuce. The last thing you want on your Burger King burger is someone's foot fungus, but as it turns out, that might be what you get. A 4channer uploaded a photo anonymously to the site showcasing his feet in a plastic bin of lettuce, with the statement: \"This is the lettuce you eat at Burger King.\" Admittedly, he had shoes on but, that's even worse. The post went live at 11:38PM on July 16 and a mere 20 minutes later, the Burger King in question was alerted to the rogue employee. at least, I hope he's rogue. How did it happen? Well, the BK employee hadn't removed the Exif data from the uploaded photo, which suggested the culprit was somewhere in Mayfield Heights, Ohio. This was at 11:47. 3 minutes later, at 11:50, the Burger King branch address was posted, with wishes of happy unemployment. 5 minutes later, the news station was contacted by another 4channer. And 3 minutes later, at 11:58, a link was posted. Bk's \"Tell Us About US\" online form. The foot photo otherwise known as Exhibit A, was attached. Cleveland Scene Magazine contacted the BK in question the next day. When questioned, the breakfast shift manager said, \"Oh, I know who that is. He's getting fired.\" Mystery, Solved, by 4chan, now we can all go back to eating our fast food in peace.";
        TracePrintf(0, "CHILD (WRITER): TtyWrite(%d, %s, %d)\n", child_tty_id, child_buf, child_len);
        TtyWrite(child_tty_id, child_buf, child_len);
        TracePrintf(0, "CHILD (WRITER): expect long speech in terminal 1");
        Exit(0);
    }
    TracePrintf(0,"PARENT (READER): Fork() rc=%d\n", rc);

    // TEST 1: parent reads before line is available
    int parent_tty_id = 0;
    int parent_len = 5;
    char *parent_buf = malloc(parent_len*sizeof(char));
    TracePrintf(0, "PARENT (READER): TtyWrite(%d, %x, %d)\n", parent_tty_id, parent_buf, parent_len);
    int actual_len = TtyRead(parent_tty_id, parent_buf, parent_len);
    TracePrintf(0,"PARENT (READER): (expect 123) parent_buf=%s\n", parent_buf);
    TracePrintf(0,"PARENT (READER): (expect 4) actual_len=%d\n", actual_len);
    Exit(0);
    return 0;
}
