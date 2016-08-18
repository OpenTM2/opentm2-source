/*
 * Created on 11.10.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 */
package transmemoptimizer;

/**
The abstract base class for all tasks with two properties: They are to be
performed as background threads and their progress needs to be measurable.
*/
abstract class Job extends Thread {
/** A short description of the sub-activity currently executing. */
  protected String currentTask;
/**
How much work the job will have done when it's finished (expressed in whatever
unit is appropriate). -1 indicates that the necessary amount of work is not
known.
*/
  protected int size = -1;
/**
How much work has already been done.
*/
  protected int status;
/**
Gets the name of the current task.
@return The current task.
*/
  String getCurrentTask() {
    return currentTask;
  }
/**
Gets the expected amount of work to be done.
@return The expected amount of work.
*/
  int getSize() {
    return size;
  }
/**
Gets the amount of work already done.
@return The amount of work already done.
*/
  int getStatus() {
    return status;
  }
/**
Decides if this job has finished its work.
@return True if the job has finished, false otherwise.
*/
  boolean isDone() {
    return status == size;
  }
}