<?php
  function fillRadioButton(&$submit, $current, $default = false) {
    if ($submit == $current) {
      return 'checked';
    }
    else {
      if ($default !== false) {
        return 'checked';
      }
    }
  }
?>
