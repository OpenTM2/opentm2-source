/*
 * Created on 11.10.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 */
package transmemoptimizer;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
/**
Provides a modal dialog to monitor the progress of a worker thread, which must
be a subclass of Job. There is a progress bar that starts out in indeterminate
mode. As soon as the expected size of the work to be done by the worker thread
is known, the progress bar is switched out of indeterminate mode. A label is
provided where the current subtask can be announced. Communication between the
dialog and the worker thread is driven by the dialog (it periodically reads the
status [how much work has been done] and current task name from the thread).
*/
class ProgressDialog extends JDialog implements ActionListener {
/** The progress bar. */
  private JProgressBar bar = new JProgressBar();
/** The job to monitor. */
  private Job job;
/** The name of the current subtask. */
  private JLabel taskName = new JLabel(" ");
/** The timer used for periodically waking up and reading information from the
job. */
  private Timer timer = new Timer(100, this);
/**
Constructs the dialog and starts the job. Also starts the timer.
@param parent The parent frame.
@param job The job to monitor.
*/
  ProgressDialog(JFrame parent, Job job) {
    super(parent, "Please wait...", true);
    this.job = job;
    setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
    bar.setIndeterminate(true);
    getContentPane().add(bar);
    getContentPane().add(taskName, BorderLayout.SOUTH);
    pack();
    setLocationRelativeTo(parent);
    job.start();
    timer.start();
    setVisible(true);
  }
/**
The action to take when the timer fires. First, update the current subtask text.
Then, if we're in indeterminate mode, see if the size of the job has become
known in the meantime. If so switch out of indeterminate mode. If we're not in
indeterminate mode, update the progress bar with the current progress of the
job. If the job has finished, stop the timer and shut down the dialog.
@param ae Not used.
*/
  public void actionPerformed(ActionEvent ae) {
    taskName.setText(job.getCurrentTask());
    if (bar.isIndeterminate()) {
      int size = job.getSize();
      if (size > -1) {
        bar.setMaximum(size);
        bar.setStringPainted(true);
        bar.setIndeterminate(false);
      }
    }
    if (!bar.isIndeterminate()) {
      bar.setValue(job.getStatus());
    }
    if (!job.isAlive()) {
      timer.stop();
      dispose();
    }
  }
}