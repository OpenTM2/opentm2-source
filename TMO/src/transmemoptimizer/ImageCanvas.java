/**
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 */


package transmemoptimizer;

import java.awt.*;
import java.io.InputStream;

/**
 * Provides an image inside of a canvas, blocking until
 * the image is loaded. Note that a modification could
 * be made to provide optional asynch image loading.
 *
 * The image may be provided or loaded as a resource of a
 * class, which is the usual technique. For example:
 * ImageCanvas canvas = new ImageCanvas(this, "Snazzy.gif");
 *
 * The canvas is sized to fit the image exactly.
 *
 * @author Jack Harich
 */
public class ImageCanvas extends Canvas {

//---------- Private Fields ------------------------------
private Image image;

//---------- Initialization ------------------------------
/**
 * Creates a Canvas containing the Image.
 */
public ImageCanvas(Image image) {
    loadImage(image);
}
/**
 * Creates a Canvas containing an Image loaded from the
 * @param resourceName of @param resourceClass.
 */
public ImageCanvas(Class resourceClass, String resourceName) {
try {
    InputStream resource =
        resourceClass.getResourceAsStream(resourceName);
    byte[] bytes = new byte[resource.available()];
    resource.read(bytes);
    Image image = Toolkit.getDefaultToolkit()
        .createImage(bytes);
    loadImage(image);

} catch(Exception ex) {
    System.out.println("ImageCanvas() - Cannot load '" + resourceName +
"' with class '" + resourceClass.getName() + "'.");
    ex.printStackTrace();
}
} // End method
//---------- Superclass Overrides ------------------------
public void paint(Graphics g) {
    g.drawImage(image,0, 0, this);
}
//---------- Private Methods -----------------------------
private void loadImage(Image image) {

    this.image = image;
    MediaTracker tracker = new MediaTracker(this);
    tracker.addImage(image, 0);

    try {
        tracker.waitForID(0);

    } catch(InterruptedException e){
        System.out.println("Cannot load image");
    }
    setSize(image.getWidth(null), image.getHeight(null));
}


} // End Class
