<?php
  define('HBF_RUNNING', 0);
  define('HBF_COMPLETED', 1);
  define('HBF_STOPPED', 2);
  define('HBF_PAUSED', 3);

  /*
   * Signal definitions. PHP used to provide these, but no longer does, so we
   * define them and hope they don't change.
   */
  define('SIGKILL', 9);
  define('SIGSTOP', 17);
  define('SIGCONT', 19);
?>
