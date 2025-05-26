void pipetest(void)
{
  struct file * pipe_a = filp_open("/tmp/test_pipe",O_CREAT | O_RDWR,0755);
    kernel_write(pipe_a,"aaaabbbccc\n",11,0);
    char test_buffer[20];
    kernel_read(pipe_a,&test_buffer,11,0);
    pr_info(test_buffer);  
}

