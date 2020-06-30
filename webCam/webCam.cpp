#include<iostream>                              //stdの利用
#include<unordered_map>                         //順序なし連想配列の利用
#include<vector>                                //Vector型の利用
#include<opencv2/opencv.hpp>                    //OprnCVの利用

#define GREEN_MINIMUM_SIZE 100                    // 領域の最小面積
#define GREEN_FLOOR cv::Scalar(30, 100, 0)      // 緑色範囲1
#define GREEN_UPPER cv::Scalar(90, 255, 255)    // 緑色範囲2

// 構造体宣言
typedef std::unordered_map<std::string, double> dict;   // 辞書型の宣言(順序無し)

// 関数宣言
cv::Mat green_detect(cv::Mat frame);            // 二値化 (緑色領域: 255, その他: 0)
std::vector<dict> blob_anarysis(cv::Mat frame); // ブロブ解析


// ラッププリント関数
template <class C>
void print(const C& c, std::ostream& os = std::cout)
{
    std::for_each(std::begin(c), std::end(c), [&os](typename C::value_type p) { os << '{' << p.first << ',' << p.second << "}, "; });
    os << std::endl;
}


// 緑色領域の抽出
cv::Mat green_detect(cv::Mat frame)
{
    cv::Mat hsv;            // frameをそのまま使うと出力もHSV画像になってしまう

    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

    cv::inRange(hsv, GREEN_FLOOR, GREEN_UPPER, frame);

    return frame;
}


// ブロブ解析
std::vector<dict> blob_anarysis(cv::Mat frame)
{
    cv::Mat labels, stats, centroids;           // 情報の登録先
    int nLab = cv::connectedComponentsWithStats(frame, labels, stats, centroids);   // 情報取得

    std::vector<cv::Mat> stat_ = {};            //stats保存用配列
    std::vector<int> areas = {};                //面積情報保存用配列

    // ラベリング情報の登録
    for (int i = 1; i < nLab; ++i)              // 背景を除くすべての面積情報を参照していく
    {
        int* statsPtr = stats.ptr<int>(i);              // stats用ポインタ
        //double* centerPtr = centroids.ptr<double>(i);   //centroids用ポインタ

        int area = statsPtr[cv::ConnectedComponentsTypes::CC_STAT_AREA];
        if (area > GREEN_MINIMUM_SIZE)            // GREEN_MINIMUM_SIZEより小さな領域は無視
        {
            stat_.push_back(stats.row(i));
            areas.push_back(area);
        }
    }

    std::vector<dict> ret = {  };
    int loop = 0;
    // 2個以下の最大値を抽出
    if (areas.size() > 1)
    {
        loop = 2;
        ret.push_back({ {"FLAG", 2} });
    }
    else if (areas.size() > 0)
    {
        loop = 1;
        ret.push_back({ {"FLAG", 1} });    
    }
    else
    {
        ret.push_back({ {"FLAG", 0} });
        return ret;
    }
    for (int i = 0; i < loop; i++)
    {
        auto detail_itr = std::max_element(areas.begin(), areas.end());     // areasに対するイテレータ
        int max_index = std::distance(areas.begin(), detail_itr);           // areas内の最大値を取得
        int* max_stats = stat_[max_index].ptr<int>(0);
        ret.push_back({     //情報を格納する
                {"left", max_stats[cv::ConnectedComponentsTypes::CC_STAT_LEFT] },       // 左上x軸
                {"top", max_stats[cv::ConnectedComponentsTypes::CC_STAT_TOP] },         // 左上y軸
                {"height", max_stats[cv::ConnectedComponentsTypes::CC_STAT_HEIGHT] },   // 領域の高さ
                {"width", max_stats[cv::ConnectedComponentsTypes::CC_STAT_WIDTH] },     // 領域の幅
                {"area", areas[max_index] }                                             // 領域面積
            } );
        areas[max_index] = 0;

    }
    return ret;
}

void debug1()//関数化バージョン
{
    /*    一枚絵の中から二つ以下の緑色領域を抽出して
          その大きさの長方形で囲むデバッグです.    */
    
          //画像を入力
    cv::Mat frame1 = cv::imread("C:\\Users\\surfa\\Desktop\\originalExe\\image\\dora.jpg");         //緑2か所
    cv::Mat frame2 = cv::imread("C:\\Users\\surfa\\Desktop\\originalExe\\image\\HLSColorSpace.png");//緑１か所
    cv::Mat frame = frame1;

    //二値化処理(マスクの作成)
    cv::Mat green = green_detect(frame);

    //ブロブ解析の結果を取得
    std::vector<dict> target = blob_anarysis(green);

    if (target[0]["FLAG"] == 2)
    {
        dict stick1 = target[1];
        dict stick2 = target[2];
        cv::rectangle(frame, cv::Rect((int)stick1["left"], (int)stick1["top"],
            (int)stick1["width"], (int)stick1["height"]), cv::Scalar(255, 0, 255), 2);
        cv::rectangle(frame, cv::Rect((int)stick2["left"], (int)stick2["top"],
            (int)stick2["width"], (int)stick2["height"]), cv::Scalar(255, 0, 255), 2);
    }
    else if (target[0]["FLAG"] == 1)
    {
        cv::rectangle(frame, cv::Rect((int)target[1]["left"], (int)target[1]["top"], 
            (int)target[1]["width"], (int)target[1]["height"]), cv::Scalar(255, 0, 255), 2);
    }

    cv::imshow("plane", frame);
    cv::imshow("green", green);

    cv::waitKey(0);
}

int webcam_debug(int camera)
{
    cv::Mat frame;
    cv::VideoCapture cam(camera);

    if (!cam.isOpened())    //正常にカメラが起動できなければ終了
    {
        return -1;
    }

    while (cam.read(frame))
    {
        //二値化処理(マスクの作成)
        cv::Mat green = green_detect(frame);

        //ブロブ解析の結果を取得
        std::vector<dict> target = blob_anarysis(green);

        if (target[0]["FLAG"] == 2)
        {
            dict stick1 = target[1];
            dict stick2 = target[2];
            cv::rectangle(frame, cv::Rect((int)stick1["left"], (int)stick1["top"],
                (int)stick1["width"], (int)stick1["height"]), cv::Scalar(255, 0, 255), 2);
            cv::rectangle(frame, cv::Rect((int)stick2["left"], (int)stick2["top"],
                (int)stick2["width"], (int)stick2["height"]), cv::Scalar(255, 0, 255), 2);
        }
        else if (target[0]["FLAG"] == 1)
        {
            cv::rectangle(frame, cv::Rect((int)target[1]["left"], (int)target[1]["top"],
                (int)target[1]["width"], (int)target[1]["height"]), cv::Scalar(255, 0, 255), 2);
        }

        cv::imshow("plane", frame);

        const int key = cv::waitKey(1);     // スレッド化したら早くなる的なの見た気がする

        if (key == 'q')     //qが押されたらカメラを閉じる
        {
            break;
        }
    }
    cv::destroyAllWindows();
    return 0;
}


int main(void)
{
    // debug1();
    webcam_debug(0);
}
