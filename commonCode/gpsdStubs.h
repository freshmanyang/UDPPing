#ifndef _GPSD_STUBS_H_
#define _GPSD_STUBS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Macro for declaring function arguments unused. */
#if defined(__GNUC__)
#  define UNUSED __attribute__((unused)) /* Flag variable as unused */
#else /* not __GNUC__ */
#  define UNUSED
#endif


#define MAXCHANNELS	72	/* must be > 12 GPS + 12 GLONASS + 2 WAAS */

typedef double timestamp_t;	/* Unix time in seconds with fractional part */

/* mode flags for setting streaming policy */
#define WATCH_ENABLE	0x000001u	/* enable streaming */
#define WATCH_DISABLE	0x000002u	/* disable watching */
#define WATCH_JSON	0x000010u	/* JSON output */
#define WATCH_NMEA	0x000020u	/* output in NMEA */
#define WATCH_RARE	0x000040u	/* output of packets in hex */
#define WATCH_RAW	0x000080u	/* output of raw packets */
#define WATCH_SCALED	0x000100u	/* scale output to floats */
#define WATCH_TIMING	0x000200u	/* timing information */
#define WATCH_DEVICE	0x000800u	/* watch specific device */
#define WATCH_NEWSTYLE	0x010000u	/* force JSON streaming */
#define WATCH_OLDSTYLE	0x020000u	/* force old-style streaming */

struct gps_fix_t {
    timestamp_t time;	/* Time of update */
    int    mode;	/* Mode of fix */
#define MODE_NOT_SEEN	0	/* mode update not seen yet */
#define MODE_NO_FIX	1	/* none */
#define MODE_2D  	2	/* good for latitude/longitude */
#define MODE_3D  	3	/* good for altitude/climb too */
    double ept;		/* Expected time uncertainty */
    double latitude;	/* Latitude in degrees (valid if mode >= 2) */
    double epy;  	/* Latitude position uncertainty, meters */
    double longitude;	/* Longitude in degrees (valid if mode >= 2) */
    double epx;  	/* Longitude position uncertainty, meters */
    double altitude;	/* Altitude in meters (valid if mode == 3) */
    double epv;  	/* Vertical position uncertainty, meters */
    double track;	/* Course made good (relative to true north) */
    double epd;		/* Track uncertainty, degrees */
    double speed;	/* Speed over ground, meters/sec */
    double eps;		/* Speed uncertainty, meters/sec */
    double climb;       /* Vertical speed, meters/sec */
    double epc;		/* Vertical speed uncertainty */
};


/*
 * Main structure that includes all previous substructures
 */

struct gps_data_t {

    timestamp_t online;		/* NZ if GPS is on line, 0 if not.*/
    void* gps_fd;
    struct gps_fix_t	fix;	/* accumulated PVT data */

    /* GPS status -- always valid */
    int    status;		/* Do we have a fix? */
#define STATUS_NO_FIX	0	/* no */
#define STATUS_FIX	1	/* yes, without DGPS */
#define STATUS_DGPS_FIX	2	/* yes, with DGPS */

   /* precision of fix -- valid if satellites_used > 0 */
    int satellites_used;        
    /* Number of satellites used in solution */


    /* redundant with the estimate elements in the fix structure */
    double epe;  /* spherical position error, 95% confidence (meters)  */

    /* satellite status -- valid when satellites_visible > 0 */
    timestamp_t skyview_time;	/* skyview timestamp */
    int satellites_visible;	/* # of satellites in view */
    double ss[MAXCHANNELS];	/* signal-to-noise ratio (dB) */


};

extern int gps_open(/*@null@*/const char *, /*@null@*/const char *,
		      /*@out@*/struct gps_data_t *);
extern int gps_close(struct gps_data_t *);
extern int gps_send(struct gps_data_t *, const char *, ... );
extern int gps_read(/*@out@*/struct gps_data_t *);
extern int gps_stream(struct gps_data_t *, unsigned int, /*@null@*/void *);
extern const char /*@null observer@*/ *gps_data(const struct gps_data_t *);
extern int gps_waiting(struct gps_data_t *, int thisWaitTimeMSs);

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif /* _GPSD_STUBS_H_ */
/* gps.h ends here */
