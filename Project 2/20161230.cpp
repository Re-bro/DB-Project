#include<stdio.h>
#include<string.h>
#include<string>
#include<iostream>
#include<fstream>
#include<vector>
#include "mysql.h"

using namespace std;

#pragma comment(lib, "libmysql.lib")

const char* host = "localhost";
const char* user = "20161230";
const char* pw = "wogud6792!";
const char* db = "my_db";

int main(void) {

	MYSQL* connection = NULL;
	MYSQL conn;
	MYSQL_RES* sql_result;
	MYSQL_ROW sql_row;

	if (mysql_init(&conn) == NULL)
		printf("mysql_init() error!");

	connection = mysql_real_connect(&conn, host, user, pw, db, 3306, (const char*)NULL, 0);
	if (connection == NULL)
	{
		printf("%d ERROR : %s\n", mysql_errno(&conn), mysql_error(&conn));
		return 1;
	}

	else
	{
		printf("Connection Succeed\n");

		if (mysql_select_db(&conn, db))
		{
			printf("%d ERROR : %s\n", mysql_errno(&conn), mysql_error(&conn));
			return 1;
		}
		string s;
		int state;

		/* Create Table */
		ifstream c_in("20161230_1.txt");
		if (!c_in) return cout << "create file not exist", 0;
		while (getline(c_in, s)) {
			state = mysql_query(connection, s.c_str());
		}
		c_in.close();

		/* Insert Tuples */
		ifstream i_in("20161230_2.txt");
		if (!i_in) return cout << "insert file not exist", 0;
		while (getline(i_in, s)) {
			state = mysql_query(connection, s.c_str());
		}
		i_in.close();

		string query;
		while (1) {
			cout << "\n---------- SELECT QUERY TYPES ----------\n\n";
			for (int i = 1; i <= 7; i++) {
				cout << "          " << i << ". TYPE " << i << '\n';
			}
			cout << "          0. QUIT\n";

			int q, sq; cin >> q;
			int ship_count = 100;
			if (q == 0) {
				break;
			}
			else if (q == 1) {
				cout << "\n------- TYPE 1 -------\n\n";
				cout << "** Assume the package shipped by USPS with tracking number X is reported to have been destroyed in an accident.\n Find the contract information for the customer. **\n";
				cout << "Which X? : ";
				string x; cin >> x;
				query = "select customer.phone_number from customer natural join(bill natural join shipment) where tracking_number = '" + x + "'";
				state = mysql_query(connection, query.c_str());
				cout << "[phone_number]\n\n";
				int cnt = 0;
				if (state == 0)
				{
					sql_result = mysql_store_result(connection);
					while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
					{
						printf("%s\n", sql_row[0]);
						cnt++;
					}
					mysql_free_result(sql_result);
				}
				if (cnt == 0) {
					cout << "Tracking number X Doesn't exist! \n";
					continue;
				}
				cout << "\n---------- Subtypes in TYPE 1 ----------\n";
				cout << "          1. TYPE 1-1\n";
				cin >> sq;
				if (sq == 1) {
					cout << "\n------- TYPE 1-1 -------\n\n";
					cout << "** Then find the contents of that shipment and create a new shipment of replacement items. **\n\n";
					query = "select tracking_number, product_count, customer_Id, payment_number, product_ID, package_ID, shipping_company from bill natural join shipment where tracking_number = '" + x + "'";
					state = mysql_query(connection, query.c_str());
					// shipment의 정보 출력
					cout << "[tracking_number / product_count / customer_id / payment_number / product_ID / package_ID / shipping_company]\n\n";
					string comp; // 배송 회사
					if (state == 0)
					{
						sql_result = mysql_store_result(connection);
						while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
						{
							printf("%s / %s / %s / %s / %s / %s / %s\n", sql_row[0], sql_row[1], sql_row[2], sql_row[3], sql_row[4], sql_row[5], sql_row[6]);
							comp = sql_row[6];
						}
						mysql_free_result(sql_result);
					}
					query = "select now()"; // 현재 시간
					state = mysql_query(connection, query.c_str());
					string date;
					if (state == 0)
					{
						sql_result = mysql_store_result(connection);
						while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
						{
							date = sql_row[0];
						}
						mysql_free_result(sql_result);
					}
					string arr_date = date; // 도착 예정 일자 7/5
					arr_date[6] = '7';
					arr_date[8] = '0';
					arr_date[9] = '5';
					string track_num = ""; // 새 추적 번호
					track_num.push_back(date[2]);
					track_num.push_back(date[3]);
					track_num.push_back(date[5]);
					track_num.push_back(date[6]);
					track_num.push_back(date[8]);
					track_num.push_back(date[9]);
					string cnt = "";
					int tmp = ship_count;
					for (int i = 1; i <= 6; i++) {
						cnt = (char)((tmp % 10) + '0') + cnt;
						tmp /= 10;
					}
					track_num += cnt;
					//새 배송 생성
					query = "insert into shipment values ('" + track_num + "', '" + comp + "', '" + date + "', '" + arr_date + "', 'NO');";
					state = mysql_query(connection, query.c_str());
					if (state == 0)
					{
						sql_result = mysql_store_result(connection);
						mysql_free_result(sql_result);
					}
					cout << "\nNew shipment of replacement items added!\n";
					//결제 정보에 새 추적 번호로 수정
					query = "update bill set tracking_number = '" + track_num + "' where tracking_number = '" + x + "'";
					state = mysql_query(connection, query.c_str());
					if (state == 0)
					{
						sql_result = mysql_store_result(connection);
						mysql_free_result(sql_result);
					}
				}
				else {
					cout << "WRONG QUERY NUMBER\n";
				}
			}
			else if (q == 2) {
				cout << "\n------- TYPE 2 -------\n";
				cout << "** Find the customer who has bought the most (by price) in the past year. **\n\n";
				query = "with product_sum(c_id, sum) as (select customer_ID, sum(product_price*product_count) from bill, product ";
				query += "where bill.product_ID = product.product_ID and bill.purchase_date between '2021-01-01 00:00:00' and '2021-12-31 23:59:59' group by customer_ID), ";
				query += "package_sum(c_id, sum) as (select customer_ID, sum(package_price*product_count) from bill, package ";
				query += "where bill.package_ID = package.package_ID and bill.purchase_date between '2021-01-01 00:00:00' and '2021-12-31 23:59:59' group by customer_ID) ";
				query += "select product_sum.c_id, (product_sum.sum + if(product_sum.c_id = package_sum.c_id, package_sum.sum, 0)) as total ";
				query += "from product_sum, package_sum order by total desc limit 1";
				state = mysql_query(connection, query.c_str());
				cout << "[customer ID /  total payment]\n\n";
				string cid = "";
				if (state == 0)
				{
					sql_result = mysql_store_result(connection);
					while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
					{
						printf("%s / %s\n", sql_row[0], sql_row[1]);
						cid = sql_row[0];
					}
					mysql_free_result(sql_result);
				}
				query = "select customer_name from customer where customer_ID = '" + cid + "'";
				state = mysql_query(connection, query.c_str());
				cout << "[customer name]\n\n";
				if (state == 0)
				{
					sql_result = mysql_store_result(connection);
					while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
					{
						printf("%s\n", sql_row[0]);
					}
					mysql_free_result(sql_result);
				}

				cout << "\n---------- Subtypes in TYPE 2 ----------\n";
				cout << "          1. TYPE 2-1\n";
				cin >> sq;
				if (sq == 1) {
					cout << "\n------- TYPE 2-1 -------\n";
					cout << "** Then find the product that the customer bought the most **\n\n";
					query = "select product_name, sum(product_count) as cnt ";
					query += "from bill, product where bill.product_ID = product.product_ID and customer_ID = '" + cid + "' ";
					query += "group by bill.product_ID order by cnt desc limit 1";
					state = mysql_query(connection, query.c_str());
					cout << "[product name / count]\n\n";
					if (state == 0)
					{
						sql_result = mysql_store_result(connection);
						while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
						{
							printf("%s / %s\n", sql_row[0], sql_row[1]);
						}
						mysql_free_result(sql_result);
					}
				}
				else {
					cout << "WRONG QUERY NUMBER\n";
				}
			}
			else if (q == 3) {
				cout << "\n------- TYPE 3 -------\n";
				cout << "** Find all products sold in the past year. **\n";
				query = "select P.product_name from product as P, bill as B ";
				query += "where P.product_ID = B.product_ID and purchase_date between '2021-01-01 00:00:00' and '2021-12-31 23:59:59' group by P.product_name ";
				state = mysql_query(connection, query.c_str());
				cout << "[product name] \n\n";
				if (state == 0)
				{
					sql_result = mysql_store_result(connection);
					while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
					{
						printf("%s\n", sql_row[0]);
					}
					mysql_free_result(sql_result);
				}

				cout << "\n---------- Subtypes in TYPE 3 ----------\n";
				cout << "          1. TYPE 3-1\n";
				cout << "          2. TYPE 3-2\n";
				cin >> sq;
				if (sq == 1) {
					cout << "\n------- TYPE 3-1 -------\n";
					cout << "** Then find the top k products by dollar-amount sold. **\n";
					cout << "Which k? : ";
					string k; cin >> k;
					query = "select P.product_name, sum(product_count*product_price) as sold from product as P, bill as B ";
					query += "where P.product_ID = B.product_ID and purchase_date between '2021-01-01 00:00:00' and '2021-12-31 23:59:59' group by P.product_name order by sold desc limit ";
					query += k;

					state = mysql_query(connection, query.c_str());
					cout << "[product name /  dollar-amount sold]\n\n";
					if (state == 0)
					{
						sql_result = mysql_store_result(connection);
						while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
						{
							printf("%s / %s \n", sql_row[0], sql_row[1]);
						}
						mysql_free_result(sql_result);
					}

				}
				else if (sq == 2) {
					cout << "\n------- TYPE 3-2 -------\n";
					cout << "** And then find the top 10% products by dollar-amount sold.\n";
					query = "select p_name, sold from(";
					query += "select p_name, sold, percent_rank() over (order by sold desc) as per from (";
					query += "select P.product_name as p_name, sum(product_count*product_price) as sold from product as P, bill as B ";
					query += "where P.product_ID = B.product_ID and purchase_date between '2021-01-01 00:00:00' and '2021-12-31 23:59:59' ";
					query += "group by P.product_name order by sold desc) T1) T2 where T2.per <= 0.1";

					state = mysql_query(connection, query.c_str());
					cout << "[product name /  dollar-amount sold]\n\n";
					if (state == 0)
					{
						sql_result = mysql_store_result(connection);
						while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
						{
							printf("%s / %s \n", sql_row[0], sql_row[1]);
						}
						mysql_free_result(sql_result);
					}
				}
				else {
					cout << "WRONG QUERY NUMBER\n";
				}
			}
			else if (q == 4) {
				cout << "\n------- TYPE 4 -------\n";
				cout << "** Find all products by unit sales in the past year. **\n";
				query = "select P.product_name from product as P, bill as B ";
				query += "where P.product_ID = B.product_ID and purchase_date between '2021-01-01 00:00:00' and '2021-12-31 23:59:59' group by P.product_name ";
				state = mysql_query(connection, query.c_str());
				cout << "[product name] \n\n";
				if (state == 0)
				{
					sql_result = mysql_store_result(connection);
					while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
					{
						printf("%s\n", sql_row[0]);
					}
					mysql_free_result(sql_result);
				}

				cout << "\n---------- Subtypes in TYPE 4 ----------\n";
				cout << "          1. TYPE 4-1\n";
				cout << "          2. TYPE 4-2\n";
				cin >> sq;
				if (sq == 1) {
					cout << "\n------- TYPE 4-1 -------\n";
					cout << "** Then find the top k products by unit sales. **\n";
					cout << "Which k? : ";
					string k; cin >> k;
					query = "select P.product_name, sum(B.product_count) as unit from product as P, bill as B ";
					query += "where P.product_ID = B.product_ID and purchase_date between '2021-01-01 00:00:00' and '2021-12-31 23:59:59' ";
					query += "group by P.product_name order by unit desc limit ";
					query += k;

					state = mysql_query(connection, query.c_str());
					cout << "[product name /  unit-sales]\n\n";
					if (state == 0)
					{
						sql_result = mysql_store_result(connection);
						while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
						{
							printf("%s / %s \n", sql_row[0], sql_row[1]);
						}
						mysql_free_result(sql_result);
					}
				}
				else if (sq == 2) {
					cout << "\n------- TYPE 4-2 -------\n";
					cout << "** And then find the top 10% products by unit sales. **\n";
					query = "select p_name, sold from(";
					query += "select p_name, sold, percent_rank() over (order by sold desc) as per from (";
					query += "select P.product_name as p_name, sum(product_count) as sold from product as P, bill as B ";
					query += "where P.product_ID = B.product_ID and purchase_date between '2021-01-01 00:00:00' and '2021-12-31 23:59:59' ";
					query += "group by P.product_name order by sold desc) T1) T2 where T2.per <= 0.1";
					state = mysql_query(connection, query.c_str());
					cout << "[product name / unit-sales]\n\n";
					if (state == 0)
					{
						sql_result = mysql_store_result(connection);
						while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
						{
							printf("%s / %s \n", sql_row[0], sql_row[1]);
						}
						mysql_free_result(sql_result);
					}
				}
				else {
					cout << "WRONG QUERY NUMBER\n";
				}
			}
			else if (q == 5) {
				cout << "\n------- TYPE 5 -------\n";
				cout << "** Find those products that are out-of-stock at every store in California. **\n";
				query = "select product_name from product natural join(product_inventory natural join inventory)";
				query += "where region = 'California' group by product_ID having sum(stock) = 0";
				state = mysql_query(connection, query.c_str());
				cout << "[product name] \n\n";
				if (state == 0)
				{
					sql_result = mysql_store_result(connection);
					while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
					{
						printf("%s \n", sql_row[0]);
					}
					mysql_free_result(sql_result);
				}

			}
			else if (q == 6) {
				cout << "\n------- TYPE 6 -------\n";
				cout << "** Find those packages that were not delivered within the promised time. **\n";
				query = "select package_name, customer_ID, promised_deliver_time, shipping_company from package natural join(bill natural join shipment) ";
				query += "where is_delivered = 'NO' and promised_deliver_time < now()";

				state = mysql_query(connection, query.c_str());
				cout << "[package_name / customer_ID / promised_deliver_time / shipping_company] \n\n";
				if (state == 0)
				{
					sql_result = mysql_store_result(connection);
					while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
					{
						printf("%s / %s / %s / %s\n", sql_row[0], sql_row[1], sql_row[2], sql_row[3]);
					}
					mysql_free_result(sql_result);
				}

			}
			else if (q == 7) {
				cout << "\n------- TYPE 7 -------\n";
				cout << "**Generate the bill for each customer for the past month. **\n";
				query = "select * from bill where year(purchase_date) = year(current_date - interval 1 month) ";
				query += "and month(purchase_date) = month(current_date - interval 1 month)";

				state = mysql_query(connection, query.c_str());
				cout << "[bill_ID / product_count / purchase_date / customer_ID / payment_number / product_ID / package_ID / tracking_number] \n\n";
				if (state == 0)
				{
					sql_result = mysql_store_result(connection);
					while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
					{
						printf("%s / %s / %s / %s / %s / %s / %s / %s\n", sql_row[0], sql_row[1], sql_row[2], sql_row[3], sql_row[4], sql_row[5], sql_row[6], sql_row[7]);
					}
					mysql_free_result(sql_result);
				}
			}
			else {
				cout << "Select query types between 0 and 7!\n";
			}
		}

		/* Delete and Drop table */
		ifstream d_in("20161230_3.txt");
		if (!d_in) return cout << "delete_drop file not exist", 0;
		while (getline(d_in, s)) {
			state = mysql_query(connection, s.c_str());
		}
		d_in.close();
		mysql_close(connection);
	}

	return 0;
}