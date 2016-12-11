/*!
  \example get_multiecho.c �����f�[�^(�}���`�G�R�[)���擾����
                           Ҫ��þ������ݣ���ز���
  \author Satofumi KAMIMURA

  $Id: get_multiecho.cpp,v a7acd36ce479 2014/05/19 06:23:12 jun $
*/

#include "Urg_driver.h"
#include "Connection_information.h"
#include <iostream>

using namespace qrk;
using namespace std;


namespace
{
    void print_echo_data(const vector<long>& data, int index,
                         int max_echo_size)
    {
        // [mm]
        for (int i = 0; i < max_echo_size; ++i) {
            cout << data[(max_echo_size * index) + i] << ", ";
        }
    }


    void print_data(const Urg_driver& urg,
                    const vector<long>& data, long time_stamp)
    {
#if 1
        // �O���̃f�[�^�݂̂�\��   ֻ��ʾ���ݵ�ǰ
        int front_index = urg.step2index(0);
        print_echo_data(data, front_index, urg.max_echo_size());
        cout << time_stamp << endl;

#else
        static_cast<void>(urg);

        // �S�Ẵf�[�^��\��        ��ʾ��������
        size_t data_n = data.size();
        cout << "# n = " << data_n << ", timestamp = " << time_stamp << endl;

        int max_echo_size = urg.max_echo_size();
        for (size_t i = 0; i < data_n; ++i) {
            print_echo_data(data, i, max_echo_size);
            cout << endl;
        }
        cout << endl;
#endif
    }
}


int main(int argc, char *argv[])
{
    Connection_information information(argc, argv);

    // �ڑ�   ����
    Urg_driver urg;
    if (!urg.open(information.device_or_ip_name(),
                  information.baudrate_or_port_number(),
                  information.connection_type())) {
        cout << "Urg_driver::open(): "
             << information.device_or_ip_name() << ": " << urg.what() << endl;
        return 1;
    }

    // �f�[�^�擾          ���ݲɼ�
    enum { Capture_times = 1 };
    urg.start_measurement(Urg_driver::Multiecho, 0, 0);
    for (int i = 0; i < Capture_times; ++i) {
        vector<long> data;
        long time_stamp = 0;

        if (!urg.get_multiecho(data, &time_stamp)) {
            cout << "Urg_driver::get_distance(): " << urg.what() << endl;
            return 1;
        }
        print_data(urg, data, time_stamp);
    }

#if defined(URG_MSC)
    getchar();
#endif
    return 0;
}
