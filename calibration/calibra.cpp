#include<opencv/cv.h>
#include<opencv/highgui.h>
#include<stdio.h>
#include<stdlib.h>

int main(){
	int board_w, board_h, n_boards, found, corner_count,j,i,success=0;
	n_boards = 5;
	board_w = 7;
	board_h = 6;
	char exit=0;
	CvCapture *capture = cvCreateCameraCapture(0);
	assert(capture);
	cvNamedWindow("Imagem",0);
	cvNamedWindow("Chessboard",0);
	cvMoveWindow("Chessboard",400,20);
	cvNamedWindow("Homography",0);
	cvMoveWindow("Homography",20,400);
	CvMat* image_points = cvCreateMat(n_boards*board_w*board_h,2,CV_32FC1);
	CvMat* object_points = cvCreateMat(n_boards*board_w*board_h,3,CV_32FC1);
	CvMat* point_counts = cvCreateMat(n_boards,1,CV_32SC1);
	CvMat* intrinsic_matrix = cvCreateMat(3,3,CV_32FC1);
	CvMat* distortion_coeffs = cvCreateMat(4,1,CV_32FC1);
	CvMat* rotation = cvCreateMat(n_boards,3,CV_32FC1);
	CvMat* translation = cvCreateMat(n_boards,3,CV_32FC1);
	CvPoint2D32f* corners = (CvPoint2D32f*) malloc (sizeof(CvPoint2D32f)*board_w*board_h);
	IplImage *image = cvQueryFrame(capture);
	IplImage *gray = cvCreateImage(cvGetSize(image),8,1);
	while((success<n_boards)){
		image = cvQueryFrame(capture);
		cvShowImage("Imagem",image);
		exit=cvWaitKey(20);
		if(exit==27) return 0;
		if(exit == 'g'){
			cvCvtColor(image, gray, CV_BGR2GRAY);
			found = cvFindChessboardCorners(
					gray, cvSize(board_w,board_h), corners, &corner_count,
					CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
			if(found != 0){
				for(i=success*board_w*board_h, j=0;j<board_w*board_h;i++,j++){
					CV_MAT_ELEM(*image_points, float,i,0) = corners[j].x;
					CV_MAT_ELEM(*image_points, float,i,1) = corners[j].y;
					CV_MAT_ELEM(*object_points,float,i,0) = j/board_w;
					CV_MAT_ELEM(*object_points,float,i,1) = j%board_w;
					CV_MAT_ELEM(*object_points,float,i,2) = 0.0f;
				}
				CV_MAT_ELEM(*point_counts, int,success,0) = corner_count;
				success++;
				printf("Success: %d %d %d %d\n",success,found, corner_count, board_w*board_h);
			}
			cvFindCornerSubPix(gray, corners, corner_count,
					cvSize(11,11),cvSize(-1,-1), cvTermCriteria(
					CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
			cvDrawChessboardCorners(image, cvSize(board_w,board_h), corners,corner_count, found);
			cvShowImage("Chessboard",image);
		}
	}
	CV_MAT_ELEM( *intrinsic_matrix, float, 0, 0 ) = 1.0f;
	CV_MAT_ELEM( *intrinsic_matrix, float, 1, 1 ) = 1.0f;
	//CALIBRATE THE CAMERA!
	cvCalibrateCamera2(
		object_points, image_points,
		point_counts, cvGetSize( image ),
		intrinsic_matrix, distortion_coeffs,
		rotation, translation,0);
	cvSave("Intrinsics.xml", intrinsic_matrix);
	cvSave("Distortion.xml", distortion_coeffs);
	cvSave("Rotarion.xml", rotation);
	cvSave("Translation.xml", translation);

	CvMat *homography = cvCreateMat(3,3,CV_32FC1);
	IplImage *photo = cvCreateImage(cvGetSize(image),IPL_DEPTH_8U,3);
	CvMat *image_points1 = cvCreateMat(board_w*board_h,2,CV_32FC1);
	CvPoint2D32f fixed_points[4];
	CvPoint2D32f photo_points[4];
	float z = 1.0f;
	while(1){
		image = cvQueryFrame(capture);
		cvShowImage("Imagem",image);
		exit=cvWaitKey(1);
		if(exit==27) return 0;
		if(exit=='g') photo = cvCloneImage(image);
		if(exit=='a') z/=2.0;
		if(exit=='z') z*=2.0;
		cvCvtColor(image, gray, CV_BGR2GRAY);
		found = cvFindChessboardCorners(
					gray, cvSize(board_w,board_h), corners, &corner_count,
					CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
		if(found != 0){
			for(i=0;i<board_w*board_h;i++){
				CV_MAT_ELEM(*image_points1, float,i,0) = corners[i].x;
				CV_MAT_ELEM(*image_points1, float,i,1) = corners[i].y;
			}
			cvFindCornerSubPix(gray, corners, corner_count,
					cvSize(11,11),cvSize(-1,-1), cvTermCriteria(
					CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
			//cvDrawChessboardCorners(image, cvSize(board_w,board_h), corners,corner_count, found);
			//cvShowImage("Chessboard",image);
			fixed_points[0].x = CV_MAT_ELEM(*image_points1, float, 0,0);
			fixed_points[0].y = CV_MAT_ELEM(*image_points1, float, 0,1);
			fixed_points[1].x = CV_MAT_ELEM(*image_points1, float, board_w-1,0);
			fixed_points[1].y = CV_MAT_ELEM(*image_points1, float, board_w-1,1);		
			fixed_points[2].x = CV_MAT_ELEM(*image_points1, float, (board_h-1)*board_w,0);
			fixed_points[2].y = CV_MAT_ELEM(*image_points1, float, (board_h-1)*board_w,1);
			fixed_points[3].x = CV_MAT_ELEM(*image_points1, float, (board_w-1)+(board_h-1)*board_w,0);
			fixed_points[3].y = CV_MAT_ELEM(*image_points1, float, (board_w-1)+(board_h-1)*board_w,1);

			cvCircle( image, cvPointFrom32f(fixed_points[0]), 9, CV_RGB(0,0,255),   3);
			cvCircle( image, cvPointFrom32f(fixed_points[1]), 9, CV_RGB(0,255,0),   3);
			cvCircle( image, cvPointFrom32f(fixed_points[2]), 9, CV_RGB(255,0,0),   3);
			cvCircle( image, cvPointFrom32f(fixed_points[3]), 9, CV_RGB(255,255,0), 3);


			photo_points[0].x = 50;
			photo_points[0].y = 50;
			photo_points[2].x = 50;
			photo_points[2].y = image->height-50;
			photo_points[1].x = image->width-50;
			photo_points[1].y = 50;
			photo_points[3].x = image->width-50;
			photo_points[3].y = image->height-50;

			cvCircle( photo, cvPointFrom32f(photo_points[0]), 9, CV_RGB(0,0,255),   3);
			cvCircle( photo, cvPointFrom32f(photo_points[1]), 9, CV_RGB(0,255,0),   3);
			cvCircle( photo, cvPointFrom32f(photo_points[2]), 9, CV_RGB(255,0,0),   3);
			cvCircle( photo, cvPointFrom32f(photo_points[3]), 9, CV_RGB(255,255,0), 3);

			cvGetPerspectiveTransform(photo_points,fixed_points,homography);

			CV_MAT_ELEM(*homography, float, 2,2) = z;

			cvWarpPerspective(
				photo,
				image,
				homography,
				0
			);

		}
		cvShowImage("Chessboard",photo);
		cvShowImage("Homography",image);
	}
}
