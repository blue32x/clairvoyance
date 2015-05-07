
public partial class MainWindow : Window
{
    private KinectSensor kinectSensor = null;
    private CoordinateMapper coordinateMapper = null;
    private MultiSourceFrameReader multiFrameSourceReader = null;

    public MainWindow()
    {
        InitializeComponent();

        // one sensor is currently supported
        this.kinectSensor = KinectSensor.GetDefault();

        // get the coordinate mapper
        this.coordinateMapper = this.kinectSensor.CoordinateMapper;

        // get the depth (display) extents
        FrameDescription colorFrameDescription = this.kinectSensor.ColorFrameSource.CreateFrameDescription(ColorImageFormat.Bgra);
        FrameDescription frameDescription = this.kinectSensor.DepthFrameSource.FrameDescription;

        // open multiframereader for depth, color, and bodyindex frames
        this.multiFrameSourceReader = this.kinectSensor.OpenMultiSourceFrameReader(FrameSourceTypes.Depth | FrameSourceTypes.Color | FrameSourceTypes.Body);

        this.multiFrameSourceReader.MultiSourceFrameArrived += this.Reader_MultiSourceFrameArrived;
    }

    private void Reader_MultiSourceFrameArrived(object sender, MultiSourceFrameArrivedEventArgs e)
    {
        MultiSourceFrame multiSourceFrame = e.FrameReference.AcquireFrame();

        if (multiSourceFrame != null)
        {
            using (DepthFrame depthFrame = multiSourceFrame.DepthFrameReference.AcquireFrame())
            {
                if (depthFrame != null)
                {
                    // Specified X, Y coordinate
                    // In 1920 x 1080 color frame
                    double x = 1000;
                    double y = 900;

                    FrameDescription depthFrameDescription = depthFrame.FrameDescription;
                    depthWidth = depthFrameDescription.Width;
                    depthHeight = depthFrameDescription.Height;

                    depthframeData = new ushort[depthWidth * depthHeight];
                    depthFrame.CopyFrameDataToArray(depthframeData);
                    CameraSpacePoint[] csp = new CameraSpacePoint[1920 * 1080];
                    this.coordinateMapper.MapColorFrameToCameraSpace(depthframeData, csp);

                    // Depth(Z Position) of specified coordinate
                    float DepthPosition = csp[(1920 * Convert.ToInt16(y)) + Convert.ToInt16(x)].Z;
                }
            }
        }
    }
}